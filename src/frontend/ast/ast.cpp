#include "ast.hpp"

namespace toycc
{

CompUnit::CompUnit(std::unique_ptr<Location> location, std::unique_ptr<FuncDef> func_def):
	BaseAST { ast_comunit, std::move(location) },
	m_func_def { std::move(func_def) }, m_comp_unit { nullptr }
{}

CompUnit::CompUnit(std::unique_ptr<Location> location,
				   std::unique_ptr<FuncDef> func_def,
				   std::unique_ptr<CompUnit> comp_unit)
	: BaseAST{ast_comunit, std::move(location)},
	  m_func_def{std::move(func_def)}, m_comp_unit{std::move(comp_unit)}
{}

auto CompUnit::get_func_def() const -> const FuncDef&
{ return *m_func_def; }

auto CompUnit::get_comp_unit() const -> const CompUnit&
{ return *m_comp_unit; }

auto CompUnit::has_next_comp() const -> bool
{ return m_comp_unit != nullptr; }

}	//namespace toycc;

