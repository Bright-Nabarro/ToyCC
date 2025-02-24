#pragma once
#include <cstddef>
#include <llvm/IR/Type.h>

namespace toycc
{

class IType
{
public:
	enum Kind
	{
		llvm,
		mock1,
		mock2,
	};

	[[nodiscard]]
	auto get_kind() const
	{ return m_kind; }

	virtual
	~IType() = default;

	[[nodiscard]] virtual
	auto get_integer_bit_width() const
		-> std::size_t = 0;

	virtual
	auto is_integer_ty() const
		-> bool = 0;


protected:
	IType(Kind type_kind) :
		m_kind { type_kind }
	{}

private:
	Kind m_kind;
};

/**
 * @brief llvm::Type* 的包装类
 */
class LLVMType: public IType
{
public:
	static auto classof(const IType* itype) -> bool
	{
		return itype->get_kind() == Kind::llvm;
	}

	LLVMType(llvm::Type* llvm_type):
		IType(llvm), m_type { llvm_type }
	{
	}

	[[nodiscard]] 
	auto get_integer_bit_width() const -> std::size_t
	override
	{
		return m_type->getIntegerBitWidth();
	}

	[[nodiscard]]
	auto is_integer_ty() const -> bool
	override
	{
		return m_type->isIntegerTy();
	}

	[[nodiscard]]
	auto get_llvm_type() -> llvm::Type*
	{ return m_type; }

private:
	// 生命周期由LLVMContext管理，不需要手动析构
	// 外部也可以安全使用shared_ptr对IType进行包装
	llvm::Type* m_type;

};

}	//namespace toycc
