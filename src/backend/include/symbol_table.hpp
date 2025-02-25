#pragma once
#include <unordered_map>
#include <string_view>
#include <llvm/IR/Value.h>

namespace toycc
{

class LocalSymbolTable final
{
public:
	/**
	 * @param uppertable 指向上层块的指针
	 * @note 上层块的symbolTable由调用者管理
	 */
	LocalSymbolTable(LocalSymbolTable* upper_table = nullptr);
	~LocalSymbolTable() = default;

	/// @return 如果本作用域存在同名变量，返回false 
	auto insert(std::string_view name, llvm::Value* value) -> bool;

	[[nodiscard]]
	auto lookup(std::string_view sv, bool search_this_level = true)
		-> std::optional<llvm::Value*>;

private:
	std::unordered_map<std::string_view, llvm::Value*> m_table;
	LocalSymbolTable* m_upper;
};

}	//namespace toycc

