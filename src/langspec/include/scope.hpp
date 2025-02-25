#pragma once
#include <vector>
#include <unordered_map>
#include "type.hpp"

namespace toycc
{

class Scope
{
public:
	using SymbolTable = std::unordered_map<std::string_view, IType>;

	virtual void add_symbol(std::string_view name, IType value);
private:
	SymbolTable m_symbols;
};

class ScopeManager
{
public:
	void push_scope();
	void pop_scope();

	void add_symbol_to_current(std::string_view name, IType value);
	[[nodiscard]]
	auto lookup(std::string_view name) -> IType;
private:
	std::vector<Scope> m_scope_stack;
};

}	//namespace toycc
