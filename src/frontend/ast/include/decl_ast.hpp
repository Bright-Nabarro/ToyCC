#pragma once
#include <vector>
#include "expr_ast.hpp"

namespace toycc
{

/// ConstInitVal 	::= ConstExpr; 
class ConstInitVal: public BaseAST
{
public:
	TOYCC_AST_FILL_CLASSOF(ast_const_init_val);
	ConstInitVal(std::unique_ptr<Location> location,
				 std::unique_ptr<ConstExpr> const_expr);

	[[nodiscard]]
	auto get_const_expr() const -> const ConstExpr&;

private:
	std::unique_ptr<ConstExpr> m_const_expr;
};


/// ConstDef		::= Ident "=" ConstInitVal;
class ConstDef: public BaseAST
{
public:
	TOYCC_AST_FILL_CLASSOF(ast_const_def);
	ConstDef(std::unique_ptr<Location> location,
			 std::unique_ptr<Ident> ident,
             std::unique_ptr<ConstInitVal> const_int_val);

	[[nodiscard]]
	auto get_ident() const -> const Ident&;
	[[nodiscard]]
	auto get_const_init_val() const -> const ConstInitVal&;

private:
	std::unique_ptr<Ident> m_ident;
	std::unique_ptr<ConstInitVal> m_const_init_val;
};


///	ConstDefList 	::= /* empty */ | ConstDefList ',' ConstDef
class ConstDefList: public BaseAST
{
public:
	using Vector = std::vector<std::unique_ptr<ConstDef>>;
	TOYCC_AST_FILL_CLASSOF(ast_const_def_list);
	ConstDefList(std::unique_ptr<Location> location);
	ConstDefList(std::unique_ptr<Location> location,
				 std::unique_ptr<ConstDefList> rhs,
				 std::unique_ptr<ConstDef> ptr);

	[[nodiscard]]
	auto get_const_defs() const -> const Vector&;

	[[nodiscard]]
	auto begin() const -> Vector::const_iterator;
	[[nodiscard]]
	auto end() const -> Vector::const_iterator;

	[[nodiscard]] constexpr
	auto size() const -> std::size_t
	{ return m_const_defs.size(); }

private:
	[[nodiscard]]
	auto get_vector() -> Vector&;

	Vector m_const_defs;
};


/// ConstDecl 		::= "const" ScalarType ConstDef ConstDefList ";";
class ConstDecl: public BaseAST
{
public:
	TOYCC_AST_FILL_CLASSOF(ast_const_decl);
	ConstDecl(std::unique_ptr<Location> location,
			  std::unique_ptr<ScalarType> scalar_type,
			  std::unique_ptr<ConstDef> const_def,
			  std::unique_ptr<ConstDefList> const_def_list);

	[[nodiscard]]
	auto get_scalar_type() const -> const ScalarType&;
	[[nodiscard]]
	auto get_first_const_def() const -> const ConstDef&;
	[[nodiscard]]
	auto get_const_def_list() const -> const ConstDefList&;
	
private:
	std::unique_ptr<ScalarType> m_scalar_type;
	std::unique_ptr<ConstDef> m_const_def;
	std::unique_ptr<ConstDefList> m_const_def_list;
};


/// InitVal 		::= Expr;
class InitVal: public BaseAST
{
public:
	TOYCC_AST_FILL_CLASSOF(ast_init_val);
	InitVal(std::unique_ptr<Location> location, std::unique_ptr<Expr> expr);
	[[nodiscard]]
	auto get_expr() const -> const Expr&;
private:
	std::unique_ptr<Expr> m_expr;
};


/// VarDef 			::= Ident | Ident "=" InitVal;
class VarDef: public BaseAST
{
public:
	TOYCC_AST_FILL_CLASSOF(ast_var_def);
	VarDef(std::unique_ptr<Location> location, std::unique_ptr<Ident> ident);
	VarDef(std::unique_ptr<Location> location, std::unique_ptr<Ident> ident,
		   std::unique_ptr<InitVal> init_val);

	auto is_initialized() const -> bool;
	auto get_ident() const -> Ident&;
	auto get_init_val() const -> InitVal&;
	
private:
	bool m_initialized;
	std::unique_ptr<Ident> m_ident;
	std::unique_ptr<InitVal> m_init_val;
};


/// VarDefList		::= /* empty */ | VarDefList ',' VarDef ;
class VarDefList: public BaseAST
{
public:
	TOYCC_AST_FILL_CLASSOF(ast_var_def_list);
	using Vector = std::vector<std::unique_ptr<VarDef>>;
	VarDefList(std::unique_ptr<Location> location);
	VarDefList(std::unique_ptr<Location> location,
			   std::unique_ptr<VarDefList> var_def_list,
			   std::unique_ptr<VarDef> var_def);

	[[nodiscard]]
	auto begin() const -> Vector::const_iterator
	{ return m_var_defs.cbegin(); }

	[[nodiscard]]
	auto end() const -> Vector::const_iterator
	{ return m_var_defs.cend(); }

	[[nodiscard]] constexpr
	auto size() const -> std::size_t
	{ return m_var_defs.size(); }

	auto get_var_defs() -> const Vector&;
private:
	Vector m_var_defs;
};


/// VarDecl 		::= ScalarType VarDef VarDefList ";";
class VarDecl: public BaseAST
{
public:
	TOYCC_AST_FILL_CLASSOF(ast_var_decl);
	VarDecl(std::unique_ptr<Location> location,
			std::unique_ptr<ScalarType> scalar_type,
			std::unique_ptr<VarDef> var_def,
			std::unique_ptr<VarDefList> var_def_list);
	
	auto get_scalar_type() const -> const ScalarType&;
	auto get_var_def() const -> const VarDef&;
	auto get_var_def_list() const -> const VarDefList&;
	
private:
	std::unique_ptr<ScalarType> m_scalar_type;
	std::unique_ptr<VarDef> m_var_def;
    std::unique_ptr<VarDefList> m_var_def_list;
};


/// Decl			::=  ConstDecl;
class Decl: public BaseAST
{
public:
	TOYCC_AST_FILL_CLASSOF(ast_decl);
	using ConstDeclPtr = std::unique_ptr<ConstDecl>;
	using VarDeclPtr = std::unique_ptr<VarDecl>;
	using Variant = std::variant<ConstDeclPtr, VarDeclPtr>;
	Decl(std::unique_ptr<Location> location,
		 ConstDeclPtr const_decl);
	Decl(std::unique_ptr<Location> location,
		 VarDeclPtr var_decl);

	[[nodiscard]]
	auto has_const_decl() const -> bool;
	[[nodiscard]]
	auto has_var_decl() const -> bool;

	[[nodiscard]]
	auto get_const_decl() const -> const ConstDecl&;
	[[nodiscard]]
	auto get_var_decl() const -> const VarDecl&;

private:
	Variant m_value;
};

}	//namespace toycc

