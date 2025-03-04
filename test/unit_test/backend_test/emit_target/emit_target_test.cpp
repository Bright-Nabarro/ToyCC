#include <gtest/gtest.h>
#include <llvm/Target/TargetMachine.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/CodeGen/CommandFlags.h>
#include <llvm/MC/TargetRegistry.h>
#include "emit_target.hpp"

using namespace toycc;

class EmitTargetTest : public ::testing::Test
{
protected:
	std::shared_ptr<spdlog::async_logger> test_logger;
	std::shared_ptr<spdlog::sinks::stdout_color_sink_st> sink;
	std::unique_ptr<llvm::TargetMachine> tm;

	void SetUp() override
	{
		sink = std::make_shared<spdlog::sinks::stdout_color_sink_st>();
		test_logger = std::make_shared<spdlog::async_logger>(
			"front", sink, spdlog::thread_pool(),
			spdlog::async_overflow_policy::block);
		tm = create_target_machine();
		//tm = nullptr;
	}

	auto create_target_machine() -> std::unique_ptr<llvm::TargetMachine>
	{
		// 三元组包括: 架构, 供应商, 操作系统环境
		auto triple = llvm::Triple{llvm::sys::getDefaultTargetTriple()};

		// 初始化目标选项, 优化代码选项
		auto target_options =
			llvm::codegen::InitTargetOptionsFromCodeGenFlags(triple);

		// 获取用户指定的CPU和特性字符串 (sse2, avx)
		auto cpu_str = llvm::codegen::getCPUStr();
		auto feature_str = llvm::codegen::getFeaturesStr();

		std::string error_str;
		// 查找目标架构如`x86_64`
		auto target = llvm::TargetRegistry::lookupTarget(
			llvm::codegen::getMArch(), triple, error_str);
		if (!target)
		{
			spdlog::error("{}", error_str);
			return nullptr;
		}

		// 创建目标机器
		// getTriple 返回三元组字符串表示
		// 指定目标的重定位模型：静态，动态(位置无关)
		auto tm =
			std::unique_ptr<llvm::TargetMachine>(target->createTargetMachine(
				triple.getTriple(), cpu_str, feature_str, target_options,
				std::optional<llvm::Reloc::Model>{
					llvm::codegen::getRelocModel()}));

		return tm;
	}
};

TEST_F (EmitTargetTest, Constructor)
{
	EmitTarget emit{ "test", tm.get(), false, "O0", test_logger };
}

TEST_F (EmitTargetTest, get_target_type)
{
	EmitTarget emit{ "test", tm.get(), false, "O0", test_logger };
}

