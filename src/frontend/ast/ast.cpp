#include "ast.hpp"

namespace toycc
{

CompUnit::CompUnit(std::unique_ptr<Location> location,
				   std::unique_ptr<Module> module)
	: BaseAST{ast_comunit, std::move(location)}, m_module{std::move(module)}
{}

[[nodiscard]]
auto CompUnit::get_module() const -> const Module&
{
	return *m_module;
}

}	//namespace toycc;

