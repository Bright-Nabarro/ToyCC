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
	enum CompUnitType {
		extern_global_variable,
		extern_func,
		static_global_variable,
		static_func,
	};
	TOYCC_AST_FILL_CLASSOF(ast_comunit)
	CompUnit(std::unique_ptr<Location> location,
			 CompUnitType type,
			 std::unique_ptr<FuncDef> func_def);

	CompUnit(std::unique_ptr<Location> location,
			 CompUnitType type,
			 std::unique_ptr<CompUnit> comp_unit,
			 std::unique_ptr<FuncDef> func_def);

	CompUnit(std::unique_ptr<Location> location,
			 CompUnitType type,
			 std::unique_ptr<Ident> ident);

	CompUnit(std::unique_ptr<Location> location,
			 CompUnitType type,
			 std::unique_ptr<CompUnit> comp_unit,
			 std::unique_ptr<Ident> ident);

	[[nodiscard]]
	auto get_func_def() const -> const FuncDef&;
	[[nodiscard]]
	auto get_comp_unit() const -> const CompUnit&;
	[[nodiscard]]
	auto has_next_comp() const -> bool;
	[[nodiscard]]
	auto get_type() const -> CompUnitType
	{ return m_type; }
	

private:
	CompUnitType m_type;
	std::unique_ptr<CompUnit> m_comp_unit;
	std::unique_ptr<FuncDef> m_func_def;
	std::unique_ptr<Ident> m_ident;
};


#undef BINARY_EXPR_FILL_CONSTRUCTORS

} //namespace toycc

