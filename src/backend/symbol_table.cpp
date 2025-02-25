#include "symbol_table.hpp"

namespace toycc
{

LocalSymbolTable::LocalSymbolTable(LocalSymbolTable* upper_table):
	m_upper { upper_table }
{}

auto LocalSymbolTable::insert(llvm::Value* value) -> bool
{
	auto [_, success] = m_table.emplace(value->getName(), value);

	return success;
}


[[nodiscard]]
auto LocalSymbolTable::lookup(std::string_view sv, bool search_this_level)
	-> std::optional<llvm::Value*>
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
	return std::nullopt;
}

}	//namespace toycc

