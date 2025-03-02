#include <gtest/gtest.h>
#include "symbol_table.hpp"

using namespace toycc;

TEST(SymbolEntryTest, Constructor)
{
	llvm::Value* value = nullptr;
	SymbolEntry entry1 { value };
	EXPECT_TRUE(entry1.is_eval);
	
	llvm::Value* alloca = nullptr;
	SymbolEntry entry2 { alloca };
	EXPECT_FALSE(entry2.is_eval);
}
