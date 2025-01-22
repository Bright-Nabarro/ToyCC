#pragma once
#include <memory>
#include <cassert>
#include <easylog.hpp>
#include "base_ast.hpp"
#include "stmt_ast.hpp"
#include "decl_ast.hpp"

namespace tinyc
{

class CompUnit: public BaseAST
{
public:
	TINYC_AST_FILL_CLASSOF(ast_comunit)
	CompUnit(std::unique_ptr<Location> location, std::unique_ptr<FuncDef> func_def);

	[[nodiscard]]
	auto get_func_def() const -> const FuncDef&;

private:
	std::unique_ptr<FuncDef> m_func_def;
};


#undef BINARY_EXPR_FILL_CONSTRUCTORS

} //namespace tinyc

