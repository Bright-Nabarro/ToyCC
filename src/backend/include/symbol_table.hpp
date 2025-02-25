#pragma once
#include <unordered_map>
#include <string_view>
#include <llvm/IR/Value.h>

namespace toycc
{


class LocalSymbolTable final
{
public:
	LocalSymbolTable(LocalSymbolTable* upper_table = nullptr);
	~LocalSymbolTable() = default;

	auto insert(llvm::Value* value) -> bool;


	[[nodiscard]]
	auto lookup(std::string_view sv, bool search_this_level) -> std::optional<llvm::Value*>;

private:
	std::unordered_map<std::string_view, llvm::Value*> m_table;
	//作用域由外部管理
	LocalSymbolTable* m_upper;
};

}	//namespace toycc

