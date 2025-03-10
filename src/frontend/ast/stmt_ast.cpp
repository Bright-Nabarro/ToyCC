#include "stmt_ast.hpp"

namespace toycc
{

SelectStmt::SelectStmt(std::unique_ptr<Location> location,
			std::unique_ptr<Expr> expr,
			std::unique_ptr<Stmt> if_stmt):
	BaseAST { ast_select_stmt, std::move(location) },
	m_expr { std::move(expr) },
	m_if_stmt { std::move(if_stmt) },
	m_else_stmt { nullptr }
{}

SelectStmt::SelectStmt(std::unique_ptr<Location> location,
			std::unique_ptr<Expr> expr,
			std::unique_ptr<Stmt> if_stmt,
			std::unique_ptr<Stmt> else_stmt):
	BaseAST { ast_select_stmt, std::move(location) },
	m_expr { std::move(expr) },
	m_if_stmt { std::move(if_stmt) },
	m_else_stmt { std::move(else_stmt) }
{}

SelectStmt::~SelectStmt() {}

auto SelectStmt::get_expr() const -> const Expr&
{
	return *m_expr;
}

auto SelectStmt::get_if_stmt() const -> const Stmt&
{
	assert(m_if_stmt != nullptr);
	return *m_if_stmt;
}

auto SelectStmt::get_else_stmt() const -> const Stmt&
{
	assert(m_else_stmt != nullptr);
	return *m_else_stmt;
}

/// Stmt
Stmt::~Stmt() {}

Stmt::Stmt(std::unique_ptr<Location> location, StmtType type):
	BaseAST { ast_stmt, std::move(location) }, m_type { type },
		m_lval { nullptr }, m_expr { nullptr },
		m_block { nullptr }, m_stmt{ nullptr },
		m_select_stmt { nullptr }
{
	assert(type == StmtType::func_return || type == StmtType::expression);
}
	
Stmt::Stmt(std::unique_ptr<Location> location, StmtType type,
	 std::unique_ptr<Expr> expr):
	BaseAST { ast_stmt, std::move(location) }, m_type { type },
		m_lval { nullptr }, m_expr { std::move(expr) },
		m_block { nullptr }, m_stmt { nullptr },
		m_select_stmt { nullptr }
{
	assert(type == StmtType::func_return || type == StmtType::expression);
}

Stmt::Stmt(std::unique_ptr<Location> location, StmtType type,
           std::unique_ptr<LVal> lval, std::unique_ptr<Expr> expr)
    : BaseAST{ast_stmt, std::move(location)}, m_type{type},
      m_lval{std::move(lval)}, m_expr{std::move(expr)},
      m_block{nullptr}, m_stmt { nullptr },
	  m_select_stmt { nullptr }
{
	assert(type == StmtType::assign);
}

Stmt::Stmt(std::unique_ptr<Location> location, StmtType type,
		   std::unique_ptr<Block> block)
	: BaseAST{ast_stmt, std::move(location)}, m_type{type}, m_lval{nullptr},
	  m_expr{nullptr}, m_block{std::move(block)}, m_stmt { nullptr },
	  m_select_stmt { nullptr }
{
	assert(type == StmtType::block);
}

Stmt::Stmt(std::unique_ptr<Location> location, StmtType type,
	std::unique_ptr<Expr> expr, std::unique_ptr<Stmt> stmt)
	: BaseAST{ast_stmt, std::move(location)}, m_type{type}, m_lval{nullptr},
	  m_expr{std::move(expr)}, m_block{}, m_stmt { std::move(stmt) },
	  m_select_stmt { nullptr }
{
	assert(type == StmtType::if_stmt || type == StmtType::while_stmt);
}


Stmt::Stmt(std::unique_ptr<Location> location, StmtType type,
	std::unique_ptr<SelectStmt> select_stmt)
	: BaseAST{ast_stmt, std::move(location)}, m_type{type}, m_lval{nullptr},
	  m_block{}, m_stmt {},
	  m_select_stmt { std::move(select_stmt) }
{
	assert(type == StmtType::if_stmt);
}


auto Stmt::has_expr() const -> bool
{
	assert(m_type == Stmt::expression || m_type == Stmt::func_return);
	return m_expr != nullptr;
}

auto Stmt::get_lval() const -> const LVal&
{
	assert(m_lval && "Stmt does not contain a LVal");
	return *m_lval;
}

auto Stmt::get_expr() const -> const Expr&
{
	assert(m_expr && "Stmt does not contain an Expr");
	return *m_expr;
}

auto Stmt::get_block() const -> const Block&
{
	assert(m_block && "Stmt does not contain a Block");
	return *m_block;
}	

auto Stmt::get_stmt() const -> const Stmt&
{
	assert(m_stmt != nullptr);
	return *m_stmt;
}

auto Stmt::get_select_stmt() const -> const SelectStmt&
{
	assert(m_select_stmt != nullptr);
	return *m_select_stmt;
}

/// Param
Param::Param(std::unique_ptr<Location> location, std::unique_ptr<ScalarType> type,
	  std::unique_ptr<Ident> id)
	: BaseAST { ast_param, std::move(location)}, m_type{std::move(type)}, m_id{std::move(id)}
{
}

auto Param::get_type() const -> const ScalarType&
{ return *m_type; }

auto Param::get_ident() const -> const Ident&
{ return *m_id; }


/// ParamList
ParamList::ParamList(std::unique_ptr<Location> location, Vector params)
	: BaseAST{ast_paramlist, std::move(location)}, m_params{std::move(params)}
{
}

auto ParamList::begin() const -> Vector::const_iterator
{
	return m_params.cbegin();
}

auto ParamList::end() const -> Vector::const_iterator
{
	return m_params.cend();
}

auto ParamList::get_params() const -> const Vector&
{
	return m_params;
}

void ParamList::add_param(std::unique_ptr<Param> param)
{
	m_params.push_back(std::move(param));
}

/// BlockItem
BlockItem::BlockItem(std::unique_ptr<Location> location, DeclPtr decl)
	: BaseAST{ast_block_item, std::move(location)}, m_value{std::move(decl)}
{
}

BlockItem::BlockItem(std::unique_ptr<Location> location, StmtPtr stmt)
	: BaseAST{ast_block_item, std::move(location)}, m_value{std::move(stmt)}
{
}

auto BlockItem::has_decl() const -> bool
{
	return std::holds_alternative<DeclPtr>(m_value);
}

auto BlockItem::has_stmt() const -> bool
{
	return std::holds_alternative<StmtPtr>(m_value);
}

auto BlockItem::get_decl() const -> const Decl&
{
	return *std::get<DeclPtr>(m_value); // 假设调用时 BlockItem 确定包含 Decl
}

auto BlockItem::get_stmt() const -> const Stmt&
{
	return *std::get<StmtPtr>(m_value); // 假设调用时 BlockItem 确定包含 Stmt
}

/// BlockItemList
BlockItemList::BlockItemList(std::unique_ptr<Location> location)
	: BaseAST{ast_block_item_list, std::move(location)}, m_block_items{}
{
}

BlockItemList::BlockItemList(std::unique_ptr<Location> location,
							 std::unique_ptr<BlockItem> block_item,
							 std::unique_ptr<BlockItemList> block_item_list)
	: BaseAST{ast_block_item_list, std::move(location)}, m_block_items{}
{
	m_block_items.push_back(std::move(block_item));
	m_block_items.insert(
		m_block_items.end(),
		std::make_move_iterator(block_item_list->m_block_items.begin()),
		std::make_move_iterator(block_item_list->m_block_items.end()));
}

auto BlockItemList::begin() const -> Vector::const_iterator
{
	return m_block_items.cbegin();
}

auto BlockItemList::end() const -> Vector::const_iterator
{
	return m_block_items.cend();
}

auto BlockItemList::get_vector() -> Vector&
{
	return m_block_items;
}


/// Block
Block::Block(std::unique_ptr<Location> location,
			 std::unique_ptr<BlockItemList> block_item_list)
	: BaseAST{ast_block, std::move(location)},
	  m_block_item_list{std::move(block_item_list)}
{
}

auto Block::get_block_item_list() const -> const BlockItemList&
{
	return *m_block_item_list;
}


/// FuncDef
FuncDef::FuncDef(std::unique_ptr<Location> location, std::unique_ptr<BuiltinType> type,
				 std::unique_ptr<Ident> ident,
				 std::unique_ptr<ParamList> paramlist,
				 std::unique_ptr<Block> block)
	:

	  BaseAST{ast_funcdef, std::move(location)}, m_type{std::move(type)},
	  m_ident{std::move(ident)}, m_paramlist{std::move(paramlist)},
	  m_block{std::move(block)}
{
}

auto FuncDef::get_type() const -> const BuiltinType&
{
	return *m_type;
}

auto FuncDef::get_ident() const -> const Ident&
{
	return *m_ident;
}

auto FuncDef::get_paramlist() const -> const ParamList&
{
	return *m_paramlist;
}

auto FuncDef::get_block() const -> const Block&
{
	return *m_block;
}

Module::Module(std::unique_ptr<Location> location, ModuleType type,
				   std::unique_ptr<FuncDef> func_def)
	: BaseAST{ast_module, std::move(location)}, m_type{type},
	  m_comp_unit{nullptr}, m_func_def{std::move(func_def)}, m_ident{nullptr}
{}

Module::Module(std::unique_ptr<Location> location,
			 ModuleType type,
			 std::unique_ptr<Module> comp_unit,
			 std::unique_ptr<FuncDef> func_def)
	: BaseAST{ast_module, std::move(location)}, m_type{type},
	  m_comp_unit{std::move(comp_unit)}, m_func_def{std::move(func_def)}, m_ident{nullptr}
{}

Module::Module(std::unique_ptr<Location> location,
			 ModuleType type,
			 std::unique_ptr<Ident> ident)
	: BaseAST{ast_module, std::move(location)}, m_type{type},
	  m_comp_unit{}, m_func_def{}, m_ident{std::move(ident)}
{}


Module::Module(std::unique_ptr<Location> location,
			 ModuleType type,
			 std::unique_ptr<Module> comp_unit,
			 std::unique_ptr<Ident> ident)
	: BaseAST{ast_comunit, std::move(location)}, m_type{type},
	  m_comp_unit{std::move(comp_unit)}, m_func_def{}, m_ident{std::move(ident)}
{}

auto Module::get_func_def() const -> const FuncDef&
{
	assert(m_func_def);
	return *m_func_def;
}

auto Module::get_module() const -> const Module&
{
	assert(m_comp_unit);
	return *m_comp_unit;
}

auto Module::has_next_module() const -> bool
{
	return m_comp_unit != nullptr;
}

}	//namespace toycc
