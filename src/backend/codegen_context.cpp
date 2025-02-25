#include "codegen_context.hpp"

namespace toycc
{

CodeGenContext::CodeGenContext(std::string_view module_name,
		std::unique_ptr<llvm::TargetMachine> target_machine):
	m_context { std::make_unique<llvm::LLVMContext>() },
	m_module { std::make_unique<llvm::Module>(module_name, *m_context) },
	m_builder { *m_context },
	m_target_machine { std::move(target_machine) }

{
}

auto CodeGenContext::get_llvm_context() -> llvm::LLVMContext&
{
	return *m_context;
}

auto CodeGenContext::get_module() -> llvm::Module*
{
	return m_module.get();
}

auto CodeGenContext::get_builder() -> llvm::IRBuilder<>&
{
	return m_builder;
}

auto CodeGenContext::get_target_machine() -> llvm::TargetMachine*
{
	return m_target_machine.get();
}

}	//namespace toycc
