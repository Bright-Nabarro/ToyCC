#include <gtest/gtest.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include "symbol_table.hpp"

using namespace toycc;
using namespace std;

TEST(GlobalSymbolTableTest, InsertAndFind)
{
    GlobalSymbolTable gtable;

    // 测试用例 1: 插入一个 eval_value 类型的条目并查找
    llvm::Value* dummyValue = reinterpret_cast<llvm::Value*>(0x1234); // 模拟 LLVM Value
    auto evalEntry = std::make_shared<SymbolEntry>(SymbolEntry::eval_value, dummyValue);
    std::string_view name1 = "eval_symbol";

    EXPECT_TRUE(gtable.insert(name1, evalEntry)); // 插入应该成功
    auto foundEntry1 = gtable.find(name1);
    EXPECT_NE(foundEntry1, nullptr); // 查找不应为空
    EXPECT_EQ(foundEntry1->type, SymbolEntry::eval_value); // 类型应匹配
    EXPECT_EQ(foundEntry1->value, dummyValue); // 值应匹配

    // 测试用例 2: 插入一个 alloca_value 类型的条目并查找
    llvm::AllocaInst* dummyAlloca = reinterpret_cast<llvm::AllocaInst*>(0x5678); // 模拟 AllocaInst
    auto allocaEntry = std::make_shared<SymbolEntry>(dummyAlloca);
    std::string_view name2 = "alloca_symbol";

    EXPECT_TRUE(gtable.insert(name2, allocaEntry)); // 插入应该成功
    auto foundEntry2 = gtable.find(name2);
    EXPECT_NE(foundEntry2, nullptr); // 查找不应为空
    EXPECT_EQ(foundEntry2->type, SymbolEntry::alloca_value); // 类型应匹配
    EXPECT_EQ(foundEntry2->alloca, dummyAlloca); // 值应匹配

    // 测试用例 3: 查找不存在的符号
    std::string_view name3 = "non_existent_symbol";
    auto foundEntry3 = gtable.find(name3);
    EXPECT_EQ(foundEntry3, nullptr); // 应返回 nullptr

    // 测试用例 4: 重复插入相同名称
    auto anotherEvalEntry = std::make_shared<SymbolEntry>(SymbolEntry::eval_value, dummyValue);
    EXPECT_FALSE(gtable.insert(name1, anotherEvalEntry)); // 重复插入应失败
    auto foundEntry4 = gtable.find(name1);
    EXPECT_EQ(foundEntry4, evalEntry); // 仍应返回第一次插入的条目
}

class LocalSymbolTableTest: public ::testing::Test
{
protected:
	llvm::LLVMContext context;
	llvm::Module* m = nullptr;
	llvm::IRBuilder<>* builder;
	llvm::FunctionType* void_func_ty = nullptr;
	llvm::Function* func = nullptr;

	int limit = 4;
	vector<llvm::Value*> values;
	vector<llvm::AllocaInst*> allocas;
	std::shared_ptr<GlobalSymbolTable> gtable;

	void SetUp() override
	{
		assert(limit >= 3);
		m = new llvm::Module{"test_module", context};
		builder = new llvm::IRBuilder<>(context);
		void_func_ty =
			llvm::FunctionType::get(llvm::Type::getVoidTy(context), false);
		func = llvm::Function::Create(
			void_func_ty, llvm::GlobalValue::ExternalLinkage, "test", m);

		llvm::BasicBlock* entry =
			llvm::BasicBlock::Create(context, "entry", func);
		builder->SetInsertPoint(entry);

		// 创建示例变量
		for (int i = 0; i < limit; ++i)
		{
			llvm::Value* val =
				llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), i);
			values.push_back(val);
		}

		// 创建 `alloca` 变量
		for (int i = 0; i < limit; ++i)
		{
			llvm::AllocaInst* alloca =
				builder->CreateAlloca(llvm::Type::getInt32Ty(context));
			allocas.push_back(alloca);
		}

		gtable = std::make_shared<GlobalSymbolTable>();
	}

	void TearDown() override
	{
		delete builder;
		delete m;
		values.clear();
		allocas.clear();
	}

	auto get_ventry(llvm::Value* value) -> std::shared_ptr<SymbolEntry>
	{
		return std::make_shared<SymbolEntry>(SymbolEntry::eval_value, value);
	}

	auto get_aentry(llvm::AllocaInst* alloca) -> std::shared_ptr<SymbolEntry>
	{
		return std::make_shared<SymbolEntry>(alloca);
	}
};

TEST_F (LocalSymbolTableTest, Constructor)
{
	LocalSymbolTable outter { func, gtable.get() };
	LocalSymbolTable inner { &outter };
	ASSERT_EQ(outter.get_func(), func);
	EXPECT_EQ(inner.get_func(), func);
	LocalSymbolTable inner2 { &inner };
	LocalSymbolTable inner3 { &inner2 };
	LocalSymbolTable inner4 { &inner3 };
	LocalSymbolTable inner5 { &inner4 };
	LocalSymbolTable inner6 { &inner5 };
	EXPECT_EQ(inner2.get_func(), func);
	EXPECT_EQ(inner3.get_func(), func);
	EXPECT_EQ(inner4.get_func(), func);
	EXPECT_EQ(inner5.get_func(), func);
	EXPECT_EQ(inner6.get_func(), func);
}

TEST_F(LocalSymbolTableTest, ThisScopeRootValue)
{
    LocalSymbolTable lv0 { func, gtable.get() };
    array<string, 3> names = { "a", "b", "c" };

    // 确保初始状态 lookup 失败
    for (const auto& name : names)
    {
        EXPECT_FALSE(lv0.lookup(name));
    }

    // 插入 `llvm::Value*`
    for (size_t i = 0; i < names.size(); ++i)
    {
        EXPECT_TRUE(lv0.insert(names[i], get_ventry(values[i])));
        auto ret = lv0.lookup(names[i]);
        ASSERT_TRUE(ret);
		EXPECT_TRUE(ret->type == SymbolEntry::eval_value);
        EXPECT_EQ(ret->value, values[i]);
    }

    // 不能重复插入相同的 `llvm::Value*`
    for (size_t i = 0; i < names.size(); ++i)
    {
        EXPECT_FALSE(lv0.insert(names[i], get_ventry(values[i])));
        auto ret = lv0.lookup(names[i]);
        ASSERT_TRUE(ret);
        EXPECT_EQ(ret->value, values[i]);
    }
}


TEST_F(LocalSymbolTableTest, ThisScopeRootAlloca)
{
    LocalSymbolTable lv0 { func, gtable.get() };
    array<string, 3> names = { "x", "y", "z" };

    // 确保初始状态 lookup 失败
    for (const auto& name : names)
    {
        EXPECT_FALSE(lv0.lookup(name));
    }

    // 插入 `llvm::AllocaInst*`
    for (size_t i = 0; i < names.size(); ++i)
    {
        EXPECT_TRUE(lv0.insert(names[i], get_aentry(allocas[i])));
        auto ret = lv0.lookup(names[i]);
        ASSERT_TRUE(ret);
        EXPECT_EQ(ret->value, allocas[i]);
    }

    // 不能重复插入相同的 `llvm::AllocaInst*`
    for (size_t i = 0; i < names.size(); ++i)
    {
        EXPECT_FALSE(lv0.insert(names[i], get_aentry(allocas[i])));
        auto ret = lv0.lookup(names[i]);
        ASSERT_TRUE(ret);
        EXPECT_EQ(ret->value, allocas[i]);
    }
}


TEST_F(LocalSymbolTableTest, ThisScopeMix)
{
    LocalSymbolTable lv0 { func, gtable.get() };

    // 变量名称
    string value_name = "v";
    string alloca_names[2] = { "a1", "a2" };

    // **Step 1: 在 `lv0` 作用域插入变量**
    EXPECT_TRUE(lv0.insert(value_name, get_ventry(values.at(0))));
    EXPECT_TRUE(lv0.insert(alloca_names[0], get_aentry(allocas.at(0))));
    EXPECT_TRUE(lv0.insert(alloca_names[1], get_aentry(allocas.at(1))));

    LocalSymbolTable lv1 { &lv0 };

    // `lv1` 作用域插入相同名称的不同变量
    EXPECT_TRUE(lv1.insert(value_name, get_ventry(values.at(1))));
    EXPECT_TRUE(lv1.insert(alloca_names[0], get_aentry(allocas.at(2))));
    EXPECT_TRUE(lv1.insert(alloca_names[1], get_aentry(allocas.at(3))));

    // **Step 3: 确保 `lv1` 查找优先找到自己的变量**
    auto lv1_value_ret = lv1.lookup(value_name);
    ASSERT_TRUE(lv1_value_ret);
    EXPECT_EQ(lv1_value_ret->value, values.at(1)); // `lv1` 作用域的 `values[1]`

    auto lv1_alloca_ret1 = lv1.lookup(alloca_names[0]);
    ASSERT_TRUE(lv1_alloca_ret1);
    EXPECT_EQ(lv1_alloca_ret1->value, allocas.at(2)); // `lv1` 作用域的 `allocas[2]`

    auto lv1_alloca_ret2 = lv1.lookup(alloca_names[1]);
    ASSERT_TRUE(lv1_alloca_ret2);
    EXPECT_EQ(lv1_alloca_ret2->value, allocas.at(3)); // `lv1` 作用域的 `allocas[3]`
}

TEST_F(LocalSymbolTableTest, UpperTableSearch)
{
    LocalSymbolTable lv0 { func, gtable.get() };

    // 变量名称
    string value_name = "v";
    string alloca_names[2] = { "a1", "a2" };

    // **Step 1: 在 `lv0` 作用域插入变量**
    EXPECT_TRUE(lv0.insert(value_name, get_ventry(values.at(0))));
    EXPECT_TRUE(lv0.insert(alloca_names[0], get_aentry(allocas.at(0))));
    EXPECT_TRUE(lv0.insert(alloca_names[1], get_aentry(allocas.at(1))));

    // **Step 2: 进入 `lv1` 作用域**
    {
        LocalSymbolTable lv1 { &lv0 };

        // `lv1` 作用域插入一个新变量，并让另一个变量查找 `lv0`
        EXPECT_TRUE(lv1.insert(value_name, get_ventry(values.at(1))));
        EXPECT_TRUE(lv1.insert(alloca_names[0], get_aentry(allocas.at(2))));
        // `alloca_names[1]` 不插入，让它查找 `lv0`

        // **Step 3: 进入 `lv2` 作用域**
        {
            LocalSymbolTable lv2 { &lv1 };

            // `lv2` 作用域插入 `value_name`，但 `alloca_names[0]` 和 `alloca_names[1]` 需要查找上层
            EXPECT_TRUE(lv2.insert(value_name, get_ventry(values.at(2))));
            // `alloca_names[0]` 不插入，应该从 `lv1` 查找
            // `alloca_names[1]` 也不插入，应该从 `lv0` 查找

            // **Step 4: 验证 `lv2` 作用域查找**
            auto lv2_value_ret = lv2.lookup(value_name);
            ASSERT_TRUE(lv2_value_ret);
            EXPECT_EQ(lv2_value_ret->value, values.at(2)); // `lv2` 本作用域

            auto lv2_alloca_ret1 = lv2.lookup(alloca_names[0]);
            ASSERT_TRUE(lv2_alloca_ret1);
            EXPECT_EQ(lv2_alloca_ret1->value, allocas.at(2)); // `lv1` 的值

            auto lv2_alloca_ret2 = lv2.lookup(alloca_names[1]);
            ASSERT_TRUE(lv2_alloca_ret2);
            EXPECT_EQ(lv2_alloca_ret2->value, allocas.at(1)); // `lv0` 的值
        }

        // **Step 5: 验证 `lv1` 作用域查找**
        auto lv1_value_ret = lv1.lookup(value_name);
        ASSERT_TRUE(lv1_value_ret);
        EXPECT_EQ(lv1_value_ret->value, values.at(1)); // `lv1` 本作用域

        auto lv1_alloca_ret1 = lv1.lookup(alloca_names[0]);
        ASSERT_TRUE(lv1_alloca_ret1);
        EXPECT_EQ(lv1_alloca_ret1->value, allocas.at(2)); // `lv1` 本作用域

        auto lv1_alloca_ret2 = lv1.lookup(alloca_names[1]);
        ASSERT_TRUE(lv1_alloca_ret2);
        EXPECT_EQ(lv1_alloca_ret2->value, allocas.at(1)); // `lv0` 的值
    }

    // **Step 6: 确保 `lv0` 作用域仍然正确**
    auto lv0_value_after = lv0.lookup(value_name);
    ASSERT_TRUE(lv0_value_after);
    EXPECT_EQ(lv0_value_after->value, values.at(0)); // `lv0` 自己的值

    auto lv0_alloca_after1 = lv0.lookup(alloca_names[0]);
    ASSERT_TRUE(lv0_alloca_after1);
    EXPECT_EQ(lv0_alloca_after1->value, allocas.at(0)); // `lv0` 自己的值

    auto lv0_alloca_after2 = lv0.lookup(alloca_names[1]);
    ASSERT_TRUE(lv0_alloca_after2);
    EXPECT_EQ(lv0_alloca_after2->value, allocas.at(1)); // `lv0` 自己的值
}

