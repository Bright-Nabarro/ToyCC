#pragma once
#include <memory>
#include <cassert>
#include "base_ast.hpp"
#include "stmt_ast.hpp"
#include "decl_ast.hpp"

namespace toycc
{

class CompUnit: public BaseAST
{
public:
	TOYCC_AST_FILL_CLASSOF(ast_comunit);
	CompUnit(std::unique_ptr<Location> location,
			 std::unique_ptr<Module> module);

	[[nodiscard]]
	auto get_module() const -> const Module&;
private:
	std::unique_ptr<Module> m_module;
};

#undef BINARY_EXPR_FILL_CONSTRUCTORS

} //namespace toycc

