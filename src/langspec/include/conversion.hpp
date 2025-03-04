#pragma once
#include <expected>
#include <system_error>
#include <memory>
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

enum class ConversionStatus
{
	success = 0,
	warning,
	failure
};

struct ConversionResult
{
	ConversionStatus status;
	llvm::Type* result_type = nullptr;
	std::error_code ec;

    // 辅助构造函数
	static auto success(llvm::Type* type) -> ConversionResult
	{
		return {ConversionStatus::success, type, utils::conversion_error::none};
	}

	static auto warning(llvm::Type* type, utils::conversion_error desc)
		-> ConversionResult
	{
		return {ConversionStatus::warning, type, desc};
	}

	static auto failure(utils::conversion_error desc) -> ConversionResult
	{
		return {ConversionStatus::failure, nullptr, desc};
	}

	void set_success(llvm::Type* type);
	void set_warning(llvm::Type* type, utils::conversion_error desc);
	void set_failure(utils::conversion_error desc);

};

struct ConversionConfig
{
#define CVT_KIND(kind, msg) \
	ConversionStatus kind##_status = ConversionStatus::success; \
	auto kind##_failure() -> bool \
	{ return kind##_status == ConversionStatus::failure; } \
	auto kind##_warning() -> bool \
	{ return kind##_status == ConversionStatus::warning; } \
	auto kind##_success() -> bool \
	{ return kind##_status == ConversionStatus::success; }
	
#include "conversion.def"
#undef CVT_KIND

	[[nodiscard]]
	static auto apply_conversion_policy(ConversionStatus status, llvm::Type* type,
								 utils::conversion_error desc)
	{
		switch(status)
		{
		case ConversionStatus::success:
			return ConversionResult::success(type);
		case ConversionStatus::warning:
			return ConversionResult::warning(type, desc);
		case ConversionStatus::failure:
			return ConversionResult::failure(desc);
		default:
			assert(false);
		}

	}
};



/**
 * @brief 处理隐式转换的类
 * @note 依据 https://zh.cppreference.com/w/c/language/conversion \
 *       通过参数的特定位置隐式标识
 */
class ConversionHelper {
public:
    /**
     * @brief 构造函数
     * @param config 配置对象
     * @param context LLVM 上下文
     */
    ConversionHelper(std::shared_ptr<ConversionConfig> config,
                     llvm::LLVMContext& context):
		m_config { config },
		m_context { context }
	{}

	virtual ~ConversionHelper() = default;

    /**
     * @brief 值变换
     * @details 处理赋值语句中的隐式转换
     * @param left 等式左边的值类型
     * @param right 等式右边的值类型，或完成算术转换后的类型
     * @return  转换后的类型或错误码
     */
    [[nodiscard]]
    auto value_conversion(llvm::Type* left, llvm::Type* right)
        -> ConversionResult;

    /**
     * @brief 通常算术转换
     * @details 处理等号右边不同类型运算时的隐式转换
     * @param left 根据优先级，运算符左边的类型
     * @param right 根据优先级，运算符右边的类型
     * @return 转换后的类型或错误码
     */
    [[nodiscard]]
    auto arithmetic_conversion(llvm::Type* left, llvm::Type* right)
        -> ConversionResult;

	/**
	 * @brief 查询类型转换到bool是否合法
	 */
	[[nodiscard]]
	auto convert_to_bool(llvm::Type* left) -> bool;

protected:
    /**
     * @brief 算数表达式中使用的整型提升, 寻找两个类型之间的较大者返回
     * @param left 左操作数类型
     * @param right 右操作数类型
     * @return 提升后的类型，或错误码
     */
    auto arithmetic_int_promotion(llvm::Type* left, llvm::Type* right) const
        -> ConversionResult;

    /**
     * @brief 赋值语句中使用的整型转换(包括窄化,提升,有符号到无符号等)，
     * @param left 期望转换成的目标类型
     * @param right 被转换的类型
     * @return 转换后的类型，或错误码
     */
    auto value_int_conversions(llvm::IntegerType* left, llvm::IntegerType* right) const
        -> ConversionResult;

	/**
	 * @brief 查询浮点是否能够合法转换为整型
	 */
	auto int2float(llvm::Type* left, llvm::Type* right) const
		-> ConversionResult;

private:
    std::shared_ptr<ConversionConfig> m_config; // 配置对象
    llvm::LLVMContext& m_context;               // LLVM 上下文
};

}	//toycc


