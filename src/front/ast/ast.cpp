#include "ast.hpp"

namespace tinyc
{

CompUnit::CompUnit(std::unique_ptr<Location> location, std::unique_ptr<FuncDef> func_def):
	BaseAST { ast_comunit, std::move(location) },
	m_func_def { std::move(func_def) }
{}

auto CompUnit::get_func_def() const -> const FuncDef&
{ return *m_func_def; }


}	//namespace tinyc;

