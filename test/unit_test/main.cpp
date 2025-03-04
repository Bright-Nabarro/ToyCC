#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/InitializePasses.h>
#include <llvm/PassRegistry.h>
#include <llvm/CodeGen/DIE.h>
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/TargetParser/Triple.h>
#include <llvm/CodeGen/CommandFlags.h>
#include <llvm/MC/TargetRegistry.h>

static llvm::codegen::RegisterCodeGenFlags CGF;
auto main(int argc, char* argv[]) -> int
{	
	// 初始化LLVM的目标支持组件
	llvm::InitLLVM X(argc, argv);
	llvm::InitializeAllTargetInfos();
	llvm::InitializeAllTargets();
	llvm::InitializeAllTargetMCs();
	llvm::InitializeAllAsmPrinters();
	llvm::InitializeAllAsmParsers();
	spdlog::init_thread_pool(8192, 1);
	llvm::cl::ParseCommandLineOptions(argc, argv,
									  "Simple LLVM CommandLine Example\n");

	testing::InitGoogleTest(&argc, argv);
	testing::InitGoogleMock(&argc, argv);
	return RUN_ALL_TESTS();
}
