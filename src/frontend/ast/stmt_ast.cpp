#include "stmt_ast.hpp"

namespace toycc
{
/// BranchStmt

template<typename OpenOrClosedStmt>
BranchStmt<OpenOrClosedStmt>::~BranchStmt() {}

template<typename OpenOrClosedStmt>
auto BranchStmt<OpenOrClosedStmt>::get_type() -> BranchType
{
	return m_br_type;
}

template<typename OpenOrClosedStmt>
auto BranchStmt<OpenOrClosedStmt>::get_expr() -> const Expr&
{
	assert(m_expr);
	return *m_expr;
}

template<typename OpenOrClosedStmt>
auto BranchStmt<OpenOrClosedStmt>::get_first_stmt() -> const ClosedStmt&
{
	assert(m_first_stmt);
	return *m_first_stmt;
}

template<typename OpenOrClosedStmt>
auto BranchStmt<OpenOrClosedStmt>::get_last_stmt() -> const OpenOrClosedStmt&
{
	assert(m_last_stmt);
	return *m_last_stmt;
}

template class BranchStmt<OpenStmt>;
template class BranchStmt<ClosedStmt>;	

ClosedStmt::ClosedStmt(std::unique_ptr<Location> location, BranchType br_type,
					   std::unique_ptr<SimpleStmt> simple_stmt)
	: BranchStmt<ClosedStmt>{ast_closed_stmt, std::move(location), br_type},
	  m_simple_stmt{std::move(simple_stmt)}
{
}

auto ClosedStmt::get_simple_stmt() -> const SimpleStmt&
{
	assert(m_simple_stmt);
	return *m_simple_stmt;
}

OpenStmt::OpenStmt(std::unique_ptr<Location> location, BranchType br_type,
				   std::unique_ptr<Expr> expr,
				   std::unique_ptr<Stmt> stmt):
	BranchStmt{ ast_open_stmt, std::move(location), br_type},
	BranchStmt::m_expr { std::move(expr) }
{
}

/// SimpleStmt
SimpleStmt::~SimpleStmt() {}

SimpleStmt::SimpleStmt(std::unique_ptr<Location> location, SimpleStmtType type):
	BaseAST { ast_stmt, std::move(location) }, m_type { type },
		m_lval { nullptr }, m_expr { nullptr },
		m_block { nullptr }
{
}
	
SimpleStmt::SimpleStmt(std::unique_ptr<Location> location, SimpleStmtType type,
	 std::unique_ptr<Expr> expr):
	BaseAST { ast_stmt, std::move(location) }, m_type { type },
		m_lval { nullptr }, m_expr { std::move(expr) },
		m_block { nullptr }
{
}

SimpleStmt::SimpleStmt(std::unique_ptr<Location> location, SimpleStmtType type,
           std::unique_ptr<LVal> lval, std::unique_ptr<Expr> expr)
    : BaseAST{ast_stmt, std::move(location)}, m_type{type},
      m_lval{std::move(lval)}, m_expr{std::move(expr)},
      m_block{nullptr}
{
}

SimpleStmt::SimpleStmt(std::unique_ptr<Location> location, SimpleStmtType type,
		   std::unique_ptr<Block> block)
	: BaseAST{ast_stmt, std::move(location)}, m_type{type}, m_lval{nullptr},
	  m_expr{nullptr}, m_block{std::move(block)}
{
}

auto SimpleStmt::has_expr() const -> bool
{
	assert(m_type == SimpleStmt::expression
		|| m_type == SimpleStmt::func_return
		|| m_type == SimpleStmt::assign);
	return m_expr != nullptr;
}

auto SimpleStmt::get_lval() const -> const LVal&
{
	assert(m_lval && "SimpleStmt does not contain a LVal");
	return *m_lval;
}

auto SimpleStmt::get_expr() const -> const Expr&
{
	assert(m_expr && "SimpleStmt does not contain an Expr");
	return *m_expr;
}

auto SimpleStmt::get_block() const -> const Block&
{
	assert(m_block && "SimpleStmt does not contain a Block");
	return *m_block;
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

auto BlockItem::get_stmt() const -> const SimpleStmt&
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
