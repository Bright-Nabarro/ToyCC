#include <gtest/gtest.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/DerivedTypes.h>
#include "mock/llvm_base.hpp"

#include "conversion.hpp"

using namespace toycc;

TEST(ConversionResult, SuccessConstructor)
{
	// 测试 success 辅助构造函数
	llvm::Type* type = nullptr; // 模拟一个 LLVM 类型指针
	ConversionResult result = ConversionResult::success(type);

	EXPECT_EQ(result.status, ConversionStatus::success);
	EXPECT_EQ(result.result_type, type);
	EXPECT_EQ(result.ec.value(),
			  static_cast<int>(utils::conversion_error::none));
}

TEST(ConversionResultTest, WarningConstructor)
{
	// 测试 warning 辅助构造函数
	llvm::Type* type = nullptr; // 模拟一个 LLVM 类型指针
	utils::conversion_error error = utils::conversion_error::arr2ptr_cvt;
	ConversionResult result = ConversionResult::warning(type, error);

	EXPECT_EQ(result.status, ConversionStatus::warning);
	EXPECT_EQ(result.result_type, type);
	EXPECT_EQ(result.ec.value(), static_cast<int>(error));
}

TEST(ConversionResult, FailureConstructor)
{
	// 测试 failure 辅助构造函数
	utils::conversion_error error = utils::conversion_error::int_narrowing;
	ConversionResult result = ConversionResult::failure(error);

	EXPECT_EQ(result.status, ConversionStatus::failure);
	EXPECT_EQ(result.result_type, nullptr);
	EXPECT_EQ(result.ec.value(), static_cast<int>(error));
}

TEST(ConversionResult, SetSuccess)
{
	// 测试 set_success 方法
	ConversionResult result;
	llvm::Type* type = nullptr; // 模拟一个 LLVM 类型指针
	result.set_success(type);

	EXPECT_EQ(result.status, ConversionStatus::success);
	EXPECT_EQ(result.result_type, type);
	EXPECT_EQ(result.ec.value(),
			  static_cast<int>(utils::conversion_error::none));
}

TEST(ConversionResult, SetWarning)
{
	// 测试 set_warning 方法
	ConversionResult result;
	llvm::Type* type = nullptr; // 模拟一个 LLVM 类型指针
	utils::conversion_error error = utils::conversion_error::arr2ptr_cvt;
	result.set_warning(type, error);

	EXPECT_EQ(result.status, ConversionStatus::warning);
	EXPECT_EQ(result.result_type, type);
	EXPECT_EQ(result.ec.value(), static_cast<int>(error));
}

TEST(ConversionResult, SetFailure)
{
	// 测试 set_failure 方法
	ConversionResult result;
	utils::conversion_error error = utils::conversion_error::int_narrowing;
	result.set_failure(error);

	EXPECT_EQ(result.status, ConversionStatus::failure);
	EXPECT_EQ(result.result_type, nullptr);
	EXPECT_EQ(result.ec.value(), static_cast<int>(error));
}

class ConversionHelperTest: public ::testing::Test
{
protected:
	llvm::LLVMContext context;
	std::shared_ptr<ConversionConfig> config;
	std::unique_ptr<ConversionHelper> helper;

	std::shared_ptr<MockType> int16_ty;
	std::shared_ptr<MockType> int32_ty;
	std::shared_ptr<MockType> int64_ty;
	std::shared_ptr<MockType> float_ty;

	void SetUp() override
	{
		config = std::make_shared<ConversionConfig>();
		helper = std::make_unique<ConversionHelper>(config, context);
		int16_ty = std::make_shared<MockType>(context, llvm::Type::TypeID::IntegerTyID);
		EXPECT_CALL(*int16_ty, isIntegerTy()).WillRepeatedly(testing::Return(true));
		int32_ty = std::make_shared<MockType>(context, llvm::Type::TypeID::IntegerTyID);
		EXPECT_CALL(*int32_ty, isIntegerTy()).WillRepeatedly(testing::Return(true));
		int64_ty = std::make_shared<MockType>(context, llvm::Type::TypeID::IntegerTyID);
		EXPECT_CALL(*int64_ty, isIntegerTy()).WillRepeatedly(testing::Return(true));
		float_ty = std::make_shared<MockType>(context, llvm::Type::TypeID::FloatTyID);
		EXPECT_CALL(*float_ty, isFloatingPointTy()).WillRepeatedly(testing::Return(true));
	}

	void TearDown() override
	{
	}
};

TEST_F(ConversionHelperTest, F2IValueConversion)
{
	
	config->float2int_cvt_status = ConversionStatus::success;
	auto f2i_suc = helper->value_conversion(float_ty.get(), int32_ty.get());
	EXPECT_EQ(f2i_suc.status, ConversionStatus::success);

	config->float2int_cvt_status = ConversionStatus::warning;
	auto f2i_warn = helper->value_conversion(float_ty.get(), int32_ty.get());
	EXPECT_EQ(f2i_warn.status, ConversionStatus::warning);

	config->float2int_cvt_status = ConversionStatus::failure;
	auto f2i_fail = helper->value_conversion(float_ty.get(), int32_ty.get());
	EXPECT_EQ(f2i_fail.status, ConversionStatus::failure);
}

TEST_F(ConversionHelperTest, I2IValueConversion)
{
    llvm::Type* int32_type = llvm::Type::getInt32Ty(context);
    llvm::Type* int64_type = llvm::Type::getInt64Ty(context);
    llvm::Type* int16_type = llvm::Type::getInt16Ty(context);

    // 测试相同整数类型转换(应该总是成功)
    auto same_type = helper->value_conversion(int32_type, int32_type);
    EXPECT_EQ(same_type.status, ConversionStatus::success);

    // 测试整数类型提升(promotion, int32->int64)
    config->int_promotion_status = ConversionStatus::success;
    auto promotion_suc = helper->value_conversion(int64_type, int32_type);
    EXPECT_EQ(promotion_suc.status, ConversionStatus::success);

    config->int_promotion_status = ConversionStatus::warning;
    auto promotion_warn = helper->value_conversion(int64_type, int32_type);
    EXPECT_EQ(promotion_warn.status, ConversionStatus::warning);

    config->int_promotion_status = ConversionStatus::failure;
    auto promotion_fail = helper->value_conversion(int64_type, int32_type);
    EXPECT_EQ(promotion_fail.status, ConversionStatus::failure);

    // 测试整数类型缩小(narrowing, int32->int16)
    config->int_narrowing_status = ConversionStatus::success;
    auto narrowing_suc = helper->value_conversion(int16_type, int32_type);
    EXPECT_EQ(narrowing_suc.status, ConversionStatus::success);

    config->int_narrowing_status = ConversionStatus::warning;
    auto narrowing_warn = helper->value_conversion(int16_type, int32_type);
    EXPECT_EQ(narrowing_warn.status, ConversionStatus::warning);

    config->int_narrowing_status = ConversionStatus::failure;
    auto narrowing_fail = helper->value_conversion(int16_type, int32_type);
    EXPECT_EQ(narrowing_fail.status, ConversionStatus::failure);
}


