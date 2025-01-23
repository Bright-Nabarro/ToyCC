#pragma once
#include <expected>
#include <system_error>
#include <memory>
#include <format>
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>

namespace toycc
{

namespace utils
{

enum class conversion_error
{
	none = 0,
#define CVT_KIND(kind, msg) \
	kind,
	#include "conversion.def"
#undef CVT_KIND
	unsupport_cvt
};


class conversion_category: public std::error_category
{
public:
	auto name() const noexcept -> const char*
	override
	{
		return "conversion_category";
	};

	auto message(int ev) const noexcept -> std::string
	override
	{
		switch(static_cast<conversion_error>(ev))
		{
		case conversion_error::none:
    	    return "No conversion error.";

#define CVT_KIND(kind, msg)                                                    \
	case conversion_error::kind:                                               \
		return msg "disabled";

#include "conversion.def"

#undef CVT_KIND
		case conversion_error::unsupport_cvt:
			return "unsupport cvt";

		default:
    	    return "Unknown conversion error.";
		};
	};
};


auto make_error_code(conversion_error e) -> std::error_code;

}	//namespace toycc::utils

}	//namespace toycc

namespace std
{

template <>
struct is_error_code_enum<toycc::utils::conversion_error> : true_type
{
};

}	//namespace std

namespace toycc
{

struct ConversionConfig
{
#define CVT_KIND(kind, msg) \
	bool enable_##kind = true;
#include "conversion.def"
#undef CVT_KIND	
};


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
 * @brief 处理隐式转换的抽象基类
 */
class IConversionHelper
{
public:
	virtual ~IConversionHelper() = default;

	enum Kind
	{
		llvm,
		mock1,
		mock2,
	};

	[[nodiscard]]
	auto get_kind() const -> Kind 
	{ return m_kind; }

	/**
	 * @brief 值变换
	 * @details 发生在赋值语句的隐式转换 
	 * @param left 等式左边值类型
	 * @param right 等式右边单个类型，或完成通常算数转换的值
	 * @retval 完成隐式转换的类型。 如果不合法，返回错误码
	 */
	[[nodiscard]]
	virtual auto value_conversion(std::shared_ptr<IType> left,
						  std::shared_ptr<IType> right)
		-> std::expected<std::shared_ptr<IType>, std::error_code>;

	/**
	 * @brief 通常算数转换
	 * @details 发生在等号右边，不同类型一起运算发生的隐式转换
	 * @param left 根据优先级，符号左边类型
	 * @param right 根据优先级，符号右边类型
	 * @retval 发生算数转换后的类型
	 */
	[[nodiscard]]
	virtual auto arithmetic_conversion(std::shared_ptr<IType> left,
							   std::shared_ptr<IType> right)
		-> std::expected<std::shared_ptr<IType>, std::error_code>;

protected:
	IConversionHelper(Kind kind, std::shared_ptr<ConversionConfig> config):
		m_kind { kind },
		m_config { config }
	{}

	/// @brief 整型提升
	virtual
	auto int_promotion(std::shared_ptr<IType> left, std::shared_ptr<IType> right) const
		-> std::expected<std::shared_ptr<IType>, std::error_code> = 0;

	/**
	 * @brief 整型转换
	 * @param left 等式左边等号，期望转换成为的类型
	 * @param right 等式右边等号，被转换的类型
	 */
	virtual
	auto int_cvt(std::shared_ptr<IType> left, std::shared_ptr<IType> right) const
		-> std::expected<std::shared_ptr<IType>, std::error_code> = 0;
	

private:
	Kind m_kind;

protected:
	std::shared_ptr<ConversionConfig> m_config;
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

/**
 * @brief 处理隐式转换
 * @note 依据 https://zh.cppreference.com/w/c/language/conversion \
 * 		 通过参数的特定位置隐式标识 \
 * 		 c语言没有引用，返回值一律视为右值
 */
class LLVMConversionHelper: public IConversionHelper
{
public:
	LLVMConversionHelper(Kind kind, std::shared_ptr<ConversionConfig> config,
			llvm::LLVMContext& context):
		IConversionHelper { kind, config },
		m_context { context }
	{}

	[[nodiscard]]
	static auto classof(const IConversionHelper* helper) -> bool
	{
		return helper->get_kind() == Kind::llvm;
	}

private:
	
	///**
	// * @brief 数组到指针转换
	// * @note incomplete
	// */
	//auto arr2ptr(llvm::Type* type) const
	//	-> llvm::Type*;

	auto int_promotion(std::shared_ptr<IType> left, std::shared_ptr<IType> right) const
		-> std::expected<std::shared_ptr<IType>, std::error_code>  override;

	auto int_cvt(std::shared_ptr<IType> left, std::shared_ptr<IType> right) const
		-> std::expected<std::shared_ptr<IType>, std::error_code>  override;

private:
	llvm::LLVMContext& m_context;

};


}	//toycc


