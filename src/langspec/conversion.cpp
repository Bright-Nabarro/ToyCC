#include <llvm/IR/Type.h>
#include <llvm/Support/Casting.h>
#include <llvm/IR/DerivedTypes.h>
#include <spdlog/spdlog.h>
#include "conversion.hpp"

namespace toycc
{

namespace utils
{

auto make_error_code(conversion_error e) -> std::error_code
{
	static conversion_category instance;
	return { static_cast<int>(e), instance };
};

}	//namespace toycc::utils



/*
 *	INTERFACE PUBLIC
 */

auto ConversionHelper::value_conversion(llvm::Type* left, llvm::Type* right)
	-> ConversionResult
{
	auto result = ConversionResult::failure(utils::conversion_error::unsupport_cvt);
	if (left->isIntegerTy() && right->isIntegerTy())
	{
		result = arithmetic_int_promotion(llvm::cast<llvm::IntegerType>(left),
										  llvm::cast<llvm::IntegerType>(right));
	}

	return result;
}

auto ConversionHelper::arithmetic_conversion(llvm::Type* left,
											 llvm::Type* right)
	-> ConversionResult
{
	auto result = ConversionResult::failure(utils::conversion_error::unsupport_cvt);

	if (left->isIntegerTy() && right->isIntegerTy())
	{
		result = arithmetic_int_promotion(left, right);
	}

	return result;
}

/*
 *	INTERFACE PRIVATE
 */


//auto LLVMConversionHelper::arr2ptr(llvm::Type* type) const
//		-> llvm::Type*
//{
//	if (!m_config->enable_arr2ptr_cvt)
//		return nullptr;
//	
//	return type;
//	
//}

auto ConversionHelper::arithmetic_int_promotion(llvm::Type* left,
												llvm::Type* right) const
	-> ConversionResult
{
	assert(left->isIntegerTy());
	assert(right->isIntegerTy());

	if (!m_config->enable_int_promotion)
		return ConversionResult::failure(utils::conversion_error::int_promotion);

	auto left_width = left->getIntegerBitWidth();
	auto right_width = right->getIntegerBitWidth();
	auto bigger = left_width > right_width ? left_width : right_width;

	llvm::Type* type = llvm::Type::getIntNTy(m_context, bigger);

	return ConversionResult::success(type);
}

auto ConversionHelper::value_int_conversions(llvm::IntegerType* left,
											 llvm::IntegerType* right) const
	-> ConversionResult
{
	auto left_width = left->getIntegerBitWidth();
	auto right_width = right->getIntegerBitWidth();
	
	if (left_width == right_width)
	{
		return ConversionResult::success(left);
	}
	else if (left_width > right_width)
	{
		if (m_config->enable_int_promotion)
			return ConversionResult::success(left);
		else
			return ConversionResult::failure(utils::conversion_error::int_promotion);
	}
	else
	{
		if (m_config->enable_int_narrowing)
			return ConversionResult::warning(left, utils::conversion_error::int_narrowing);
		else
			return ConversionResult::failure(utils::conversion_error::int_narrowing);
	}
	
}

}	//namespace toycc


