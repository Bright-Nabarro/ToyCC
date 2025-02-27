#pragma once
#include <unordered_map>
#include <string_view>
#include <llvm/IR/Value.h>
#include <llvm/IR/Instructions.h>

namespace toycc
{

struct SymbolEntry
{
	SymbolEntry(llvm::Value* value_):
		is_eval { true }, value { value_ }
	{}

	SymbolEntry(llvm::AllocaInst* alloca_):
		is_eval { false }, alloca { alloca_ }
	{}

	bool is_eval;
	union
	{
		llvm::Value* value;			// eval
		llvm::AllocaInst* alloca;
	};
};

class LocalSymbolTable final
{
public:
	/**
	 * @param uppertable 指向上层块的指针
	 * @note 上层块的symbolTable由调用者管理
	 */
	LocalSymbolTable(LocalSymbolTable* upper_table = nullptr);
	~LocalSymbolTable() = default;

	/**
	 * @param value 为eval声明的eval变量
	 * @return 如果本作用域存在同名变量，返回false 
	 */
	auto insert(std::string_view name, llvm::Value* value) -> bool;

	/**
	 * @param value 为eval声明的eval变量
	 * @return 如果本作用域存在同名变量，返回false 
	 */
	auto insert(std::string_view name, llvm::AllocaInst* alloca) -> bool;

	[[nodiscard]]
	auto lookup(std::string_view sv, bool search_this_level = true)
		-> std::shared_ptr<SymbolEntry>;

private:
	std::unordered_map<std::string_view, std::shared_ptr<SymbolEntry>> m_table;
	LocalSymbolTable* m_upper;
};

}	//namespace toycc

