#include <llvm/CodeGen/CommandFlags.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/Pass.h>
#include <expected>
#include "emit_target.hpp"

namespace toycc
{

EmitTarget::EmitTarget(std::string_view inputfile_name,
		   llvm::TargetMachine* target_machine, bool emit_llvm,
		   std::string optimization_level,
		   std::shared_ptr<spdlog::async_logger> logger):
	m_inputfile_name { inputfile_name }, m_target_name { std::nullopt },
	m_target_machine { target_machine }, m_emit_llvm { emit_llvm },
	m_optimization_level { optimization_level }, m_logger { logger }
{
}

EmitTarget::EmitTarget(std::string_view inputfile_name, std::string_view target_name,
		   llvm::TargetMachine* target_machine, bool emit_llvm,
		   std::string optimization_level,
		   std::shared_ptr<spdlog::async_logger> logger):
	m_inputfile_name { inputfile_name }, m_target_name { target_name },
	m_target_machine { target_machine }, m_emit_llvm { emit_llvm },
	m_optimization_level { optimization_level }, m_logger { logger }
{
}

auto EmitTarget::operator()(std::unique_ptr<llvm::Module> module)
	-> std::expected<void, std::string>
{
	std::error_code ec;
	auto open_flags = llvm::sys::fs::OF_None;
	
	auto target_name = get_target_name();
	llvm::raw_fd_ostream os { target_name, ec, open_flags };
	if (ec) [[unlikely]]
	{
		return std::unexpected{std::format("Could not open file {}: {}",
										   target_name, ec.message())};
	}

	llvm::legacy::PassManager pm;
	
	switch(get_target_type())
	{
	case llvm_ir:
		pm.add(llvm::createPrintModulePass(os));
		break;
	case assembly:
	case llvm_bin:
	case object: {
		auto error = m_target_machine->addPassesToEmitFile(
			pm, os, nullptr, llvm::codegen::getFileType());
		if (error)
		{
			return std::unexpected { "No support for file type" };
		}
		break;
	}
	case unkown:
		return std::unexpected { "operatoer() get an unknown target type" };
	default:
		assert(false && "Unhandled TargetType");
	}

	pm.run(*module);
	return {};
}

auto EmitTarget::get_target_type() -> TargetType
{
	auto file_type = llvm::codegen::getFileType();
	switch(file_type)
	{
	case llvm::CodeGenFileType::AssemblyFile:
		if (m_emit_llvm)
			return llvm_ir;
		return assembly;
	case llvm::CodeGenFileType::ObjectFile:
		if (m_emit_llvm)
			return llvm_bin;
		return object;
	case llvm::CodeGenFileType::Null:
		if (m_emit_llvm)
			return llvm_ir;
		return object;
	default:
		m_logger->error("unknown target type");
		return unkown;
	}
}

auto EmitTarget::erase_file_postfix(std::string_view file_name)
	-> std::string_view
{
	std::size_t beg = file_name.rfind('/');
	if (beg == std::string_view::npos)
		beg = 0;
	else 
		++beg; 

	std::size_t end = file_name.rfind('.');
	
	return file_name.substr(beg, end - beg);
}

auto EmitTarget::get_target_name() -> std::string
{
	if (m_target_name.has_value())
		return std::string { m_target_name.value() };

	std::string result { erase_file_postfix(m_inputfile_name) };

	switch(get_target_type())
	{
	case llvm_ir:
		result += ".ll";
		break;
	case llvm_bin:
		result += ".bc";
		break;
	case assembly:
		result += ".s";
		break;
	case object:
		result += ".o";
		break;
	case unkown:
		m_logger->error("Unknown target type encountered in EmitTarget::get_target_name. ");
	}

	return result;
}

}	//namespace toycc

