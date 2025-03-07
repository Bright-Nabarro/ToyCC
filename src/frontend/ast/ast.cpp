#include "ast.hpp"

namespace toycc
{

CompUnit::CompUnit(std::unique_ptr<Location> location, std::unique_ptr<FuncDef> func_def):
	BaseAST { ast_comunit, std::move(location) },
	m_comp_unit { nullptr }, m_func_def { std::move(func_def) }
{}

CompUnit::CompUnit(std::unique_ptr<Location> location,
				   std::unique_ptr<CompUnit> comp_unit,
				   std::unique_ptr<FuncDef> func_def)
	: BaseAST{ast_comunit, std::move(location)},
	  m_comp_unit{std::move(comp_unit)}, m_func_def{std::move(func_def)}
{}

CompUnit::CompUnit(std::unique_ptr<Location> location, CompUnitType type,
				   std::unique_ptr<FuncDef> func_def)
	: BaseAST{ast_comunit, std::move(location)}, m_type{type},
	  m_comp_unit{nullptr}, m_func_def{std::move(func_def)}, m_ident{nullptr}
{
}

CompUnit::CompUnit(std::unique_ptr<Location> location,
			 CompUnitType type,
			 std::unique_ptr<CompUnit> comp_unit,
			 std::unique_ptr<FuncDef> func_def);

CompUnit::CompUnit(std::unique_ptr<Location> location,
			 CompUnitType type,
			 std::unique_ptr<Ident> ident);

CompUnit::CompUnit(std::unique_ptr<Location> location,
			 CompUnitType type,
			 std::unique_ptr<CompUnit> comp_unit,
			 std::unique_ptr<Ident> ident);

auto CompUnit::get_func_def() const -> const FuncDef&
{
	assert(!m_func_def);
	return *m_func_def;
}

auto CompUnit::get_comp_unit() const -> const CompUnit&
{
	assert(!m_comp_unit);
	return *m_comp_unit;
}

auto CompUnit::has_next_comp() const -> bool
{
	return m_comp_unit != nullptr;
}

}	//namespace toycc;

