#include "symbol_table.hpp"

namespace toycc
{

LocalSymbolTable::LocalSymbolTable(LocalSymbolTable* upper_table):
	m_upper { upper_table }
{}

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

