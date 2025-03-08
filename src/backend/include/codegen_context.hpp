#pragma once

#include "type_mgr.hpp"
#include "conversion.hpp"
#include <memory>
#include <expected>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Support/SourceMgr.h>
#include <spdlog/async.h>

namespace toycc
{

class CodeGenContext
{
public:
	CodeGenContext(std::shared_ptr<ConversionConfig> cvt_config,
				   llvm::SourceMgr& src_mgr, std::shared_ptr<llvm::TargetMachine> tm,
				   std::shared_ptr<spdlog::async_logger> logger);

	auto get_llvm_context() -> llvm::LLVMContext&
	{
		return *m_context;
	}

	auto get_module() -> llvm::Module*
	{
		return m_module.get();
	}

	auto get_builder() -> llvm::IRBuilder<>&
	{
		return m_builder;
	}

	auto get_cvt_helper() -> ConversionHelper&
	{
		return *m_cvt_helper;
	}

	auto get_src_mgr() -> llvm::SourceMgr&
	{
		return m_src_mgr;
	}

	auto get_logger() -> spdlog::async_logger&
	{
		return *m_logger;
	}

	auto get_result() -> std::unique_ptr<llvm::Module>
	{
		return std::move(m_module);
	}
	
	auto get_type_mgr() -> TypeMgr&
	{
		return *m_type_mgr;
	}
	
private:
	std::unique_ptr<llvm::LLVMContext> m_context;
	std::unique_ptr<llvm::Module> m_module;
	llvm::IRBuilder<> m_builder;
	std::unique_ptr<TypeMgr> m_type_mgr;
	std::shared_ptr<ConversionHelper> m_cvt_helper;

	llvm::SourceMgr& m_src_mgr;
	std::shared_ptr<llvm::TargetMachine> m_target_machine;
	std::shared_ptr<spdlog::async_logger> m_logger;
};


class CGContextInterface 
{
public:
	CGContextInterface(std::shared_ptr<CodeGenContext> context):
		m_cg_context { context }
	{}

	[[nodiscard]]
	virtual auto get_llvm_context() -> llvm::LLVMContext&;
	[[nodiscard]]
	virtual auto get_module() -> llvm::Module*;
	[[nodiscard]]
	virtual auto get_builder() -> llvm::IRBuilder<>&;
	[[nodiscard]]
	virtual auto get_cvt_helper() -> ConversionHelper&;
	[[nodiscard]]
	virtual auto get_src_mgr() -> llvm::SourceMgr&;
	[[nodiscard]]
	virtual auto get_logger() -> spdlog::async_logger&;
	[[nodiscard]]
	virtual auto get_result() -> std::unique_ptr<llvm::Module>;
	[[nodiscard]]
	virtual auto get_type_mgr() -> TypeMgr&;
private:
	std::shared_ptr<CodeGenContext> m_cg_context;
};

}	//namespace toycc
