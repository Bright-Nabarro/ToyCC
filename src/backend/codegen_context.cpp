#include "codegen_context.hpp"

namespace toycc
{

CodeGenContext::CodeGenContext(
				   std::shared_ptr<ConversionConfig> cvt_config,
				   llvm::SourceMgr& src_mgr, std::shared_ptr<llvm::TargetMachine> tm,
				   std::shared_ptr<spdlog::async_logger> logger):

	m_context { std::make_unique<llvm::LLVMContext>() },
	m_module{std::make_unique<llvm::Module>("toycc.expr", *m_context)},
	m_builder{m_module->getContext()},
	m_type_mgr{std::make_unique<TypeMgr>(m_module->getContext(), tm.get())},
	m_cvt_helper { std::make_unique<ConversionHelper>(cvt_config, *m_context) },
	m_src_mgr{src_mgr}, m_target_machine{tm}, m_logger{logger}
{}

#define CGI_GETTER(func_name, return_type) \
	auto CGContextInterface::func_name() -> return_type \
	{ \
		return m_cg_context-> func_name(); \
	}

CGI_GETTER(get_llvm_context, llvm::LLVMContext&)
CGI_GETTER(get_module, llvm::Module*)
CGI_GETTER(get_builder, llvm::IRBuilder<>&)
CGI_GETTER(get_cvt_helper, ConversionHelper&)
CGI_GETTER(get_src_mgr, llvm::SourceMgr&)
CGI_GETTER(get_logger, spdlog::async_logger&)
CGI_GETTER(get_result, std::unique_ptr<llvm::Module>)
CGI_GETTER(get_type_mgr, TypeMgr&)

}	//namespace toycc
