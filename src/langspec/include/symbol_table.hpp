#pragma once
#include <unordered_map>
#include <string_view>
#include <llvm/IR/Value.h>
#include <llvm/IR/Instructions.h>

namespace toycc
{

struct SymbolEntry
{
	enum EntryType
	{
		eval_value,
		alloca_value,
		func_value,
	};

	SymbolEntry(EntryType type_, llvm::Value* value_):
		type { type_ }, value { value_ }
	{
		assert(type_ != alloca_value);
	}

	SymbolEntry(llvm::AllocaInst* alloca_):
		type { alloca_value }, alloca { alloca_ }
	{}

	
	EntryType type;
	union
	{
		llvm::Value* value;			// eval
		llvm::AllocaInst* alloca;
	};
};


class GlobalSymbolTable final
{
public:
	auto find(std::string_view name) -> std::shared_ptr<SymbolEntry>;
	auto insert(std::string_view name, std::shared_ptr<SymbolEntry> entry) -> bool;
private:
	std::unordered_map<std::string_view, std::shared_ptr<SymbolEntry>> m_table;
};


class LocalSymbolTable final
{
public:
	/**
	 * @param uppertable 指向上层块的指针
	 * @note 上层块的symbolTable由调用者管理
	 */
	LocalSymbolTable(LocalSymbolTable* upper_table);
	LocalSymbolTable(llvm::Function* func,
					 GlobalSymbolTable* global);
	~LocalSymbolTable() = default;

	/**
	 * @return 如果本作用域存在同名变量，返回false 
	 */
	auto insert(std::string_view name, std::shared_ptr<SymbolEntry> entry) -> bool;

	[[nodiscard]]
	auto lookup(std::string_view sv, bool search_this_level = true)
		-> std::shared_ptr<SymbolEntry>;

	[[nodiscard]]
	auto get_func() -> llvm::Function*
	{
		assert(m_func);
		return m_func;
	}

private:
	std::unordered_map<std::string_view, std::shared_ptr<SymbolEntry>> m_table;
	LocalSymbolTable* m_upper;
	llvm::Function* m_func;
	GlobalSymbolTable* m_global_table;
};

}	//namespace toycc

