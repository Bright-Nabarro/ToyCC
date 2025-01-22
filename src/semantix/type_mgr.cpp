#include "type_mgr.hpp"

namespace tinyc
{
TypeMgr::TypeMgr(llvm::LLVMContext& context, llvm::TargetMachine* target_machine):
	m_context { context },
	m_data_layout { target_machine->createDataLayout() },
	m_void_ty { llvm::Type::getVoidTy(context) }, 
	m_bool_ty { llvm::Type::getIntNTy(context, 8) },
	m_schar_ty { llvm::Type::getIntNTy(context, 8) },
	m_uchar_ty { llvm::Type::getIntNTy(context, 8) },
	m_sshort_ty { llvm::Type::getIntNTy(context, 16) },
	m_ushort_ty { llvm::Type::getIntNTy(context, 16) },
	m_sint_ty { llvm::Type::getIntNTy(context, 32) },
	m_uint_ty { llvm::Type::getIntNTy(context, 32) },
	m_slong_ty { llvm::Type::getIntNTy(context, gen_long_bit_width()) },
	m_ulong_ty { llvm::Type::getIntNTy(context, gen_long_bit_width()) },
	m_slong_long_ty { llvm::Type::getIntNTy(context, 64) },
	m_ulong_long_ty { llvm::Type::getIntNTy(context, 64) },
	m_float_ty { llvm::Type::getFloatTy(context) },
	m_double_ty { llvm::Type::getFloatTy(context) }
{}

#define GET_TYPE(type_name)                                                    \
	auto TypeMgr::get_##type_name() const->llvm::Type*                        \
	{                                                                          \
		return m_##type_name##_ty;                                             \
	}

#define GET_SIGN_TYPE(type_name)                                               \
	auto TypeMgr::get_signed_##type_name() const->llvm::Type*                 \
	{                                                                          \
		return m_s##type_name##_ty;                                            \
	}                                                                          \
                                                                               \
	auto TypeMgr::get_unsigned_##type_name() const->llvm::Type*               \
	{                                                                          \
		return m_u##type_name##_ty;                                            \
	}

GET_TYPE(void)

GET_TYPE(bool)
GET_SIGN_TYPE(char)
GET_SIGN_TYPE(short)
GET_SIGN_TYPE(int)
GET_SIGN_TYPE(long)
GET_SIGN_TYPE(long_long)

GET_TYPE(float)
GET_TYPE(double)

}	//namespace tinyc

