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

auto IConversionHelper::value_conversion(std::shared_ptr<IType> left,
					  std::shared_ptr<IType> right)
	-> std::expected<std::shared_ptr<IType>, std::error_code>
{
	
}


auto IConversionHelper::arithmetic_conversion(std::shared_ptr<IType> left,
						   std::shared_ptr<IType> right)
	-> std::expected<std::shared_ptr<IType>, std::error_code>
{
	std::expected<std::shared_ptr<IType>, std::error_code> result =
		std::unexpected<std::error_code>{
			utils::conversion_error::unsupport_cvt};

	if (left->is_integer_ty() && right->is_integer_ty())
	{
		result = int_promotion(left, right);
	}
	
	/*
	 *===========================================================
	 *      这里需要向日志输出错误，左值和右值的类型描述
	 *===========================================================
	 */

	
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

auto LLVMConversionHelper::int_promotion(std::shared_ptr<IType> left,
										 std::shared_ptr<IType> right) const
	-> std::expected<std::shared_ptr<IType>, std::error_code> 
{
	assert(left->is_integer_ty());
	assert(right->is_integer_ty());

	if (!m_config->enable_int_cvt)
		return std::unexpected<std::error_code>(utils::conversion_error::int_promotion);

	auto left_width = left->get_integer_bit_width();
	auto right_width = right->get_integer_bit_width();
	auto bigger = left_width > right_width ? left_width : right_width;

	std::shared_ptr<IType> ret =
		std::make_shared<LLVMType>(llvm::Type::getIntNTy(m_context, bigger));

	return ret;
}

auto LLVMConversionHelper::int_cvt(std::shared_ptr<IType> left,
								   std::shared_ptr<IType> right) const
	-> std::expected<std::shared_ptr<IType>, std::error_code>
{
	if (!m_config->enable_int_cvt)
		return std::unexpected<std::error_code>(utils::conversion_error::int_cvt);
	
	return left;
}

}	//namespace toycc


