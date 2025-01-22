#pragma once
#include <llvm/IR/Type.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Target/TargetMachine.h>

namespace tinyc
{

class TypeMgr
{
public:
	TypeMgr(llvm::LLVMContext& context, llvm::TargetMachine* target_machine);

	auto get_void() const -> llvm::Type*;
	auto get_bool() const -> llvm::Type*;
	/// @note 包括char
	auto get_signed_char() const -> llvm::Type*;
	auto get_unsigned_char() const -> llvm::Type*;
	auto get_signed_short() const -> llvm::Type*;
	auto get_unsigned_short() const -> llvm::Type*;
	auto get_signed_int() const -> llvm::Type*;
	auto get_unsigned_int() const -> llvm::Type*;
	auto get_signed_long() const -> llvm::Type*;
	auto get_unsigned_long() const -> llvm::Type*;
	auto get_signed_long_long() const -> llvm::Type*;
	auto get_unsigned_long_long() const -> llvm::Type*;

	auto get_float() const -> llvm::Type*;
	auto get_double() const -> llvm::Type*;

private:
	auto gen_long_bit_width() -> int
	{
		auto pointer_bit_width = m_data_layout.getPointerSizeInBits();

		if (pointer_bit_width == 32)
			return 32;
		else if (pointer_bit_width == 64)
			return 64;
		else
			return 32;
	}

private:
	llvm::LLVMContext& m_context;
	llvm::DataLayout m_data_layout;

	llvm::Type* m_void_ty;
	// integer
	llvm::Type* m_bool_ty;	//_Bool before c23
	llvm::Type* m_schar_ty;
	llvm::Type* m_uchar_ty;
	llvm::Type* m_sshort_ty;
	llvm::Type* m_ushort_ty;
	llvm::Type* m_sint_ty;
	llvm::Type* m_uint_ty;
	llvm::Type* m_slong_ty;
	llvm::Type* m_ulong_ty;
	llvm::Type* m_slong_long_ty;
	llvm::Type* m_ulong_long_ty;
	// floating point
	llvm::Type* m_float_ty;
	llvm::Type* m_double_ty;
};

}	//namespace tinyc

