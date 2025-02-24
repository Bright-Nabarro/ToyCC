#pragma once

#include <memory>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Target/TargetMachine.h>

namespace toycc
{

class CodeGenContext
{
public:
	CodeGenContext(std::string_view module_name,
			std::unique_ptr<llvm::TargetMachine> target_machine);

	auto get_llvm_context() -> llvm::LLVMContext&;
	auto get_module() -> llvm::Module*;
	auto get_builder() -> llvm::IRBuilder<>&;
	auto get_target_machine() -> llvm::TargetMachine*;
	
private:
	std::unique_ptr<llvm::LLVMContext> m_context;
	std::unique_ptr<llvm::Module> m_module;
	llvm::IRBuilder<> m_builder;
	std::unique_ptr<llvm::TargetMachine> m_target_machine;
};

}	//namespace toycc
