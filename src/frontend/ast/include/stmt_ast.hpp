#pragma once
#include "base_ast.hpp"
#include "expr_ast.hpp"
#include "decl_ast.hpp"
#include "base_components_ast.hpp"


namespace toycc
{

class Block;
class Stmt;

class SelectStmt: public BaseAST
{
public:
	TOYCC_AST_FILL_CLASSOF(ast_select_stmt);
	SelectStmt(std::unique_ptr<Location> location,
			std::unique_ptr<Expr> expr,
			std::unique_ptr<Stmt> if_stmt);
	SelectStmt(std::unique_ptr<Location> location,
			std::unique_ptr<Expr> expr,
			std::unique_ptr<Stmt> if_stmt,
			std::unique_ptr<Stmt> else_stmt);
	~SelectStmt();

	[[nodiscard]]
	auto has_else_stmt() const -> bool
	{ return m_else_stmt != nullptr; }

	[[nodiscard]]
	auto get_expr() const -> const Expr&;
	[[nodiscard]]
	auto get_if_stmt() const -> const Stmt&;
	[[nodiscard]]
	auto get_else_stmt() const -> const Stmt&;

private:
	std::unique_ptr<Expr> m_expr;
	std::unique_ptr<Stmt> m_if_stmt;
	std::unique_ptr<Stmt> m_else_stmt;
};

class Stmt: public BaseAST
{
public:
	TOYCC_AST_FILL_CLASSOF(ast_stmt);
	enum StmtType
	{
		assign,
		expression,
		block,
		func_return,
		if_stmt,
		while_stmt,
	};
	Stmt(std::unique_ptr<Location> location, StmtType type);

	Stmt(std::unique_ptr<Location> location, StmtType type,
		 std::unique_ptr<Expr> expr);

	Stmt(std::unique_ptr<Location> location, StmtType type,
		 std::unique_ptr<LVal> lval, std::unique_ptr<Expr> expr);

	Stmt(std::unique_ptr<Location> location, StmtType type,
		std::unique_ptr<Block> block);

	Stmt(std::unique_ptr<Location> location, StmtType type,
		std::unique_ptr<Expr> expr, std::unique_ptr<Stmt> stmt);

	Stmt(std::unique_ptr<Location> location, StmtType type,
		 std::unique_ptr<SelectStmt> select_stmt);

	~Stmt();

	[[nodiscard]]
	auto get_type() const -> StmtType
	{ return m_type; }

	[[nodiscard]]
	auto has_expr() const -> bool;

	[[nodiscard]]
	auto get_lval() const -> const LVal&;

	[[nodiscard]]
	auto get_expr() const -> const Expr&;

	[[nodiscard]]
	auto get_block() const -> const Block&;

	[[nodiscard]]
	auto get_stmt() const -> const Stmt&;

	[[nodiscard]]
	auto get_select_stmt() const -> const SelectStmt&;
private:
	StmtType m_type;
	std::unique_ptr<LVal> m_lval;
	std::unique_ptr<Expr> m_expr;
	std::unique_ptr<Block> m_block;
	std::unique_ptr<Stmt> m_stmt;
	std::unique_ptr<SelectStmt> m_select_stmt;
};


/**
 * Param ::= Type Ident; 
 **/
class Param : public BaseAST
{
public:
	Param(std::unique_ptr<Location> location, std::unique_ptr<ScalarType> type,
		  std::unique_ptr<Ident> id);

	TOYCC_AST_FILL_CLASSOF(ast_param);
	
	[[nodiscard]]
	auto get_type() const -> const ScalarType&;
	[[nodiscard]]
	auto get_ident() const -> const Ident&;
	
private:
	std::unique_ptr<ScalarType> m_type;
	std::unique_ptr<Ident> m_id;
};


/**
 * ParamList   ::= empty | Param ("," Param)* ;
 **/
class ParamList: public BaseAST
{
public:
	TOYCC_AST_FILL_CLASSOF(ast_paramlist);
	using Vector = std::vector<std::unique_ptr<Param>>;

	ParamList(std::unique_ptr<Location> location, Vector params = Vector{});

	[[nodiscard]]
	auto begin() const -> Vector::const_iterator;
	[[nodiscard]]
	auto end() const -> Vector::const_iterator;
	[[nodiscard]]
	auto get_params() const -> const Vector&;
	void add_param(std::unique_ptr<Param> param);
	
private:
	Vector m_params;

};


///BlockItem		::= Decl | Stmt;
class BlockItem: public BaseAST
{
public:
	using DeclPtr = std::unique_ptr<Decl>;
	using StmtPtr = std::unique_ptr<Stmt>;
	using Variant = std::variant<DeclPtr, StmtPtr>;

	TOYCC_AST_FILL_CLASSOF(ast_block_item);

	BlockItem(std::unique_ptr<Location> location, DeclPtr decl);
	BlockItem(std::unique_ptr<Location> location, StmtPtr stmt);

	[[nodiscard]]
	auto has_decl() const -> bool;
	[[nodiscard]]
	auto has_stmt() const -> bool;
	
	[[nodiscard]]
	auto get_decl() const -> const Decl&;
	[[nodiscard]]
	auto get_stmt() const -> const Stmt&;

private:
	Variant m_value;
};


/// BlockItemList 	::= /* empty */ | BlockItem BlockItemList
class BlockItemList : public BaseAST
{
public:
	using Vector = std::vector<std::unique_ptr<BlockItem>>;
	TOYCC_AST_FILL_CLASSOF(ast_block_item_list);
	
	BlockItemList(std::unique_ptr<Location> location);
	BlockItemList(std::unique_ptr<Location> location,
				  std::unique_ptr<BlockItem> block_item,
				  std::unique_ptr<BlockItemList> block_item_list);

	auto begin() const -> Vector::const_iterator;
	auto end() const -> Vector::const_iterator;
	
	auto get_vector() -> Vector&;

private:
	Vector m_block_items;
};


/// Block       	::= "{" BlockItemList "}";
class Block: public BaseAST
{
public:
	TOYCC_AST_FILL_CLASSOF(ast_block);
	Block(std::unique_ptr<Location> location,
		  std::unique_ptr<BlockItemList> block_item_list);

	auto get_block_item_list() const -> const BlockItemList&;

private:
	std::unique_ptr<BlockItemList> m_block_item_list;
};


class FuncDef: public BaseAST
{
public:
	TOYCC_AST_FILL_CLASSOF(ast_funcdef)

	FuncDef(
		std::unique_ptr<Location> location, 
		std::unique_ptr<BuiltinType> type,
		std::unique_ptr<Ident> ident,
		std::unique_ptr<ParamList> paramlist,
		std::unique_ptr<Block> block);

	[[nodiscard]]
	auto get_type () const -> const BuiltinType&;
	[[nodiscard]]
	auto get_ident () const -> const Ident&;
	[[nodiscard]]
	auto get_paramlist () const -> const ParamList&;
	[[nodiscard]]
	auto get_block () const -> const Block&;

private:
	std::unique_ptr<BuiltinType> m_type;
	std::unique_ptr<Ident> m_ident;
	std::unique_ptr<ParamList> m_paramlist;
	std::unique_ptr<Block> m_block;
};

class Module: public BaseAST
{
public:
	enum ModuleType {
		extern_global_variable,
		extern_func,
		static_global_variable,
		static_func,
	};
	TOYCC_AST_FILL_CLASSOF(ast_module)
	Module(std::unique_ptr<Location> location,
			 ModuleType type,
			 std::unique_ptr<FuncDef> func_def);

	Module(std::unique_ptr<Location> location,
			 ModuleType type,
			 std::unique_ptr<Module> comp_unit,
			 std::unique_ptr<FuncDef> func_def);

	Module(std::unique_ptr<Location> location,
			 ModuleType type,
			 std::unique_ptr<Ident> ident);

	Module(std::unique_ptr<Location> location,
			 ModuleType type,
			 std::unique_ptr<Module> comp_unit,
			 std::unique_ptr<Ident> ident);

	[[nodiscard]]
	auto get_func_def() const -> const FuncDef&;
	[[nodiscard]]
	auto get_module() const -> const Module&;
	[[nodiscard]]
	auto has_next_module() const -> bool;
	[[nodiscard]]
	auto get_type() const -> ModuleType
	{ return m_type; }
	

private:
	ModuleType m_type;
	std::unique_ptr<Module> m_comp_unit;
	std::unique_ptr<FuncDef> m_func_def;
	std::unique_ptr<Ident> m_ident;
};

} // namespace toycc

