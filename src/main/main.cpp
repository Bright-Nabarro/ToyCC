#include "driver.hpp"
#include "code_gen_visitor.hpp"
#include "emit_target.hpp"
#include <print>
#include <llvm/CodeGen/CommandFlags.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/TargetParser/Triple.h>
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>


//帮助codegen 生成target_options
static llvm::codegen::RegisterCodeGenFlags CGF;

/// 定义命令行选项
static llvm::cl::opt<std::string> input_file {
	llvm::cl::Positional, // 位置参数，无需用 "--" 指定
	llvm::cl::desc("<input file>"),
	llvm::cl::init("toycc.out") // 默认值为 empty
};

/// 指定输出文件
static llvm::cl::opt<std::string> output_file{
	"o", // 使用 -o 指定
	llvm::cl::desc("Specify output filename"), llvm::cl::value_desc("filename"),
};

static llvm::cl::opt<bool> emit_llvm{
	"emit-llvm", 
	llvm::cl::desc("Emit LLVM IR code instead of machine code"),
	llvm::cl::init(false)
};

/// 开启debug追踪
static llvm::cl::opt<bool> debug_trace{
	"trace",
	llvm::cl::desc("Enable flex debug trace"),
	llvm::cl::init(false)
};

/// 用户指定的目标三元组
static llvm::cl::opt<std::string> mtriple {
	"mtriple",
	llvm::cl::desc("Override target triple for module")
};

/// 开启词法分析，语法分析的过程输出
static llvm::cl::opt<bool> trace_debug {
	"trace_debug",
	llvm::cl::desc("Enable bison status shift output"),
	llvm::cl::init(false)
};

/// 优化级别
static llvm::cl::opt<std::string> optimization {
	"O",
	llvm::cl::desc("optimization level"),
	llvm::cl::init("0")
};

auto create_target_machine() -> llvm::TargetMachine*
{
	//三元组包括: 架构, 供应商, 操作系统环境
	auto triple = llvm::Triple {
		llvm::sys::getDefaultTargetTriple()
	};

	//初始化目标选项, 优化代码选项
	auto target_options =
		llvm::codegen::InitTargetOptionsFromCodeGenFlags(triple);

	//获取用户指定的CPU和特性字符串 (sse2, avx)
	auto cpu_str = llvm::codegen::getCPUStr();
	auto feature_str = llvm::codegen::getFeaturesStr();

	std::string error_str;
	//查找目标架构如`x86_64`
	auto target = llvm::TargetRegistry::lookupTarget(llvm::codegen::getMArch(), triple, error_str);
	if (!target)
	{
		spdlog::error("{}", error_str);
		return nullptr;
	}
	
	// 创建目标机器
	// getTriple 返回三元组字符串表示
	// 指定目标的重定位模型：静态，动态(位置无关)
	auto tm = target->createTargetMachine(
		triple.getTriple(), cpu_str, feature_str, target_options,
		std::optional<llvm::Reloc::Model>{llvm::codegen::getRelocModel()});

	return tm;
}

/**
 * @brief 词法分析，语法分析
 * @ret 错误返回nullptr
 */
auto lex_and_parse(llvm::SourceMgr& src_mgr, std::string_view file, auto& global_sink)
	-> std::unique_ptr<toycc::CompUnit>
{
	toycc::DriverFactory driver_factory { src_mgr };

	auto front_logger = std::make_shared<spdlog::async_logger>("front", global_sink,
		spdlog::thread_pool(), spdlog::async_overflow_policy::block);
	spdlog::register_logger(front_logger);

	auto driver_or_error = driver_factory.produce_driver(file);
	if (!driver_or_error)
	{
		front_logger->info("{}", driver_or_error.error());
		return nullptr;
	}

	auto driver = std::move(*driver_or_error);
	driver->set_trace(trace_debug);

	if (!driver->parse())
		return nullptr;

	return driver->get_ast_unique();
}

auto main(int argc, char* argv[]) -> int
{
	// 初始化LLVM的目标支持组件
	llvm::InitLLVM X(argc, argv);
	llvm::InitializeAllTargets();
	llvm::InitializeAllTargetMCs();
	llvm::InitializeAllAsmPrinters();
	llvm::InitializeAllAsmParsers();
	// 初始化spdlog
	spdlog::init_thread_pool(8192, 1);
	// 全局日志输出目标，为标注彩色输出(包括错误输出)
	auto global_sink = std::make_shared<spdlog::sinks::stdout_color_sink_st>();

	// 解析命令行选项
	llvm::cl::ParseCommandLineOptions(argc, argv,
									  "Simple LLVM CommandLine Example\n");
	auto tm = create_target_machine();
	if (tm == nullptr)
		return 1;

	llvm::LLVMContext ctx;
	// 全局的源码管理
	llvm::SourceMgr src_mgr;

	// 后端日志记录，包含主函数的日志输出
	auto backend_logger = std::make_shared<spdlog::async_logger>("backend", global_sink,
			spdlog::thread_pool(), spdlog::async_overflow_policy::block);
	spdlog::register_logger(backend_logger);

	// 词法，语法分析
	auto ast = lex_and_parse(src_mgr, input_file.getValue(), global_sink);
	if (ast == nullptr)
		return 1;

	// 语义分析，中间代码生成
	toycc::CodeGenVisitor visitor(ctx, src_mgr, tm);
	auto void_or_error = visitor.visit(ast.get());
	if (!void_or_error)
	{
		backend_logger->info("{}", void_or_error.error());
		return 1;
	}

	//生成目标文件 (llvm-ir, 汇编或二进制.o)
	toycc::EmitTarget emit{input_file.getValue(), tm, emit_llvm.getValue(),
						   optimization.getValue()};
	if (!output_file.empty())
		emit.set_target_name(output_file.getValue());

	void_or_error = emit(visitor.get_module());
	if (!void_or_error)
	{
		std::println(stderr, "{}", void_or_error.error());
		backend_logger->info("{}", void_or_error.error());
		return 1;
	}

	return 0;
}

