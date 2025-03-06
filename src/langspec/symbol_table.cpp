#include "symbol_table.hpp"

namespace toycc
{

auto GlobalSymbolTable::find(std::string_view name) -> llvm::Value*
{
	auto itr = m_table.find(name);
	if (itr == m_table.end())
		return nullptr;
	return itr->second;
}

auto GlobalSymbolTable::insert(std::string_view name, llvm::Value* n) -> bool
{
	auto [ _, success ] = m_table.emplace(name, n);
	return success;
}

LocalSymbolTable::LocalSymbolTable(LocalSymbolTable* upper_table):
	m_upper { upper_table }, m_func { upper_table->m_func }
{
	assert(upper_table->m_func != nullptr);
}

LocalSymbolTable::LocalSymbolTable(llvm::Function* func):
	m_upper { nullptr }, m_func { func }
{
}

auto LocalSymbolTable::insert(std::string_view name, llvm::Value* value) -> bool
{
	auto entry = std::make_shared<SymbolEntry>(value);
	auto [_, success] = m_table.emplace(name, entry);

	return success;
}


auto LocalSymbolTable::insert(std::string_view name, llvm::AllocaInst* alloca) -> bool
{
	auto entry = std::make_shared<SymbolEntry>(alloca);
	auto [_, success] = m_table.emplace(name, entry);

	return success;
}


[[nodiscard]]
auto LocalSymbolTable::lookup(std::string_view sv, bool search_this_level)
		-> std::shared_ptr<SymbolEntry>
{
	auto cur = search_this_level ? this : this->m_upper;
	for (; cur != nullptr; cur = cur->m_upper)
	{
		auto itr = cur->m_table.find(sv);
		if (itr != cur->m_table.end())
		{
			return itr->second;
		}
	}
	
	return nullptr;
}

}	//namespace toycc

