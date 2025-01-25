#pragma once
#include <llvm/IR/Module.h>
#include <expected>
#include <llvm/Target/TargetMachine.h>
#include <string>
#include <spdlog/spdlog.h>
#include <spdlog/async.h>

namespace toycc
{

class EmitTarget
{
public:
	enum TargetType
	{
		llvm_ir,
		llvm_bin,
		assembly,
		object,
		unkown,
	};

	EmitTarget(std::string_view inputfile_name,
			   llvm::TargetMachine* target_machine, bool emit_llvm,
			   std::string m_optimization_level,
			   std::shared_ptr<spdlog::async_logger> logger);

	EmitTarget(std::string_view inputfile_name, std::string_view target_name,
			   llvm::TargetMachine* target_machine, bool emit_llvm,
			   std::string m_optimization_level,
			   std::shared_ptr<spdlog::async_logger> logger);

	void set_target_name(std::string_view target_name)
	{ m_target_name = target_name; }

	[[nodiscard]]
	auto operator()(std::unique_ptr<llvm::Module> module)
		-> std::expected<void, std::string>;
	[[nodiscard]]
	auto get_target_type() -> TargetType;
	
private:
	[[nodiscard]] static
	auto erase_file_postfix(std::string_view file_name) -> std::string_view;

	[[nodiscard]]
	auto get_target_name() -> std::string;

private:
	std::string_view m_inputfile_name;
	std::optional<std::string_view> m_target_name;
	llvm::TargetMachine* m_target_machine;
	bool m_emit_llvm;
	std::string m_optimization_level;
	std::shared_ptr<spdlog::async_logger> m_logger;
};

}	//namespace toycc
