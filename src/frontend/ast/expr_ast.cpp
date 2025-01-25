#include <cassert>
#include "expr_ast.hpp"

namespace toycc
{


/// BaseExpr
BaseExpr::BaseExpr(AstKind ast_kind, std::unique_ptr<Location> location):
	BaseAST(ast_kind, std::move(location))
{
	assert(ast_kind >= ast_expr && ast_kind < ast_expr_end);
}

auto BaseExpr::classof(const BaseAST* ast) -> bool
{
	return ast->get_kind() >= ast_expr &&
		   ast->get_kind() < ast_op_end;
}

/// Expr
// 让unique_ptr的析构函数知道UnaryExpr的定义
Expr::~Expr() {}

Expr::Expr(std::unique_ptr<Location> location, std::unique_ptr<LowExpr> uptr)
	: BaseExpr{ ast_expr, std::move(location) }, m_value{std::move(uptr)}
{
}

auto Expr::get_low_expr() const -> const LowExpr&
{ return *m_value; }


/// ConstExpr
ConstExpr::ConstExpr(std::unique_ptr<Location> location, std::unique_ptr<Expr> expr):
	BaseExpr { ast_const_expr, std::move(location)},
	m_expr { std::move(expr) }
{
}

auto ConstExpr::get_expr() const -> const Expr&
{
	return *m_expr;
}


/// PrimaryExpr
PrimaryExpr::PrimaryExpr(std::unique_ptr<Location> location, ExprPtr expr_ptr):
	BaseExpr(ast_primary_expr, std::move(location)),
	m_value { std::move(expr_ptr) }
{}

PrimaryExpr::PrimaryExpr(std::unique_ptr<Location> location, NumberPtr number_ptr):
	BaseExpr(ast_primary_expr, std::move(location)),
	m_value { std::move(number_ptr) }
{}

PrimaryExpr::PrimaryExpr(std::unique_ptr<Location> location, LValPtr lval_ptr):
	BaseExpr(ast_primary_expr, std::move(location)),
	m_value { std::move(lval_ptr) }
{}

auto PrimaryExpr::has_expr() const -> bool
{
	return std::holds_alternative<ExprPtr>(m_value);
}

auto PrimaryExpr::has_number() const -> bool
{
	 return std::holds_alternative<NumberPtr>(m_value);
}

auto PrimaryExpr::has_ident() const -> bool
{
	return std::holds_alternative<LValPtr>(m_value);
}

auto PrimaryExpr::get_expr() const -> const Expr&
{
	return *std::get<ExprPtr>(m_value);
}

auto PrimaryExpr::get_lval() const -> const LVal&
{
	return *std::get<LValPtr>(m_value);
}

auto PrimaryExpr::get_number() const -> const Number&
{
	return *std::get<NumberPtr>(m_value);
}


/// UnaryExpr
UnaryExpr::UnaryExpr(std::unique_ptr<Location> location, PrmExpPtr primary_expr):
	BaseExpr { ast_unary_expr, std::move(location) },
	m_value { std::move(primary_expr) }
{}

UnaryExpr::UnaryExpr(std::unique_ptr<Location> location, std::unique_ptr<UnaryOp> unary_op,
		std::unique_ptr<UnaryExpr> unary_expr):
	BaseExpr { ast_unary_expr, std::move(location) }, 
	m_value { PackPtr { std::move(unary_op), std::move(unary_expr) } }
{}

auto UnaryExpr::has_primary_expr() const -> bool
{
	return std::holds_alternative<PrmExpPtr>(m_value);
}

auto UnaryExpr::has_unary_expr() const -> bool
{
	return std::holds_alternative<PackPtr>(m_value);
}

auto UnaryExpr::get_primary_expr() const -> const PrimaryExpr&
{
	assert(has_primary_expr());
	return *std::get<PrmExpPtr>(m_value);
}

auto UnaryExpr::get_unary_op() const -> const UnaryOp&
{
	assert(has_unary_expr());
	return *(std::get<PackPtr>(m_value).first);
}

auto UnaryExpr::get_unary_expr() const -> const UnaryExpr&
{
	assert(has_unary_expr());
	return *(std::get<PackPtr>(m_value).second);
}


/// BinaryExpr
template <typename SelfExpr, typename HigherExpr, typename Op>
	requires std::is_base_of_v<::toycc::Operator, Op>
BinaryExpr<SelfExpr, HigherExpr, Op>::BinaryExpr (
		AstKind kind, std::unique_ptr<Location> location, HigherExprPtr ptr)
	: BinaryExprBase{kind, std::move(location)}, m_value{std::move(ptr)}
{
}

template <typename SelfExpr, typename HigherExpr, typename Op>
	requires std::is_base_of_v<::toycc::Operator, Op>
BinaryExpr<SelfExpr, HigherExpr, Op>::BinaryExpr(
	AstKind kind, std::unique_ptr<Location> location, SelfExprPtr self_ptr,
	OpPtr op_ptr, HigherExprPtr higher_ptr)
	: BinaryExprBase{kind, std::move(location)},
	  m_value{CombinedExpr{std::move(self_ptr), std::move(op_ptr),
						   std::move(higher_ptr)}}
{
}

template <typename SelfExpr, typename HigherExpr, typename Op>
	requires std::is_base_of_v<::toycc::Operator, Op>
auto BinaryExpr<SelfExpr, HigherExpr, Op>::has_higher_expr() const
	-> bool
{
	return std::holds_alternative<HigherExprPtr>(m_value);
}

template <typename SelfExpr, typename HigherExpr, typename Op>
	requires std::is_base_of_v<::toycc::Operator, Op>
auto BinaryExpr<SelfExpr, HigherExpr, Op>::has_combined_expr() const
	-> bool
{
	return std::holds_alternative<CombinedExpr>(m_value);
}

template <typename SelfExpr, typename HigherExpr, typename Op>
	requires std::is_base_of_v<::toycc::Operator, Op>
auto BinaryExpr<SelfExpr, HigherExpr, Op>::get_higher_expr() const
	-> const HigherExpr&
{
	return *std::get<HigherExprPtr>(m_value);
}

template <typename SelfExpr, typename HigherExpr, typename Op>
	requires std::is_base_of_v<::toycc::Operator, Op>
auto BinaryExpr<SelfExpr, HigherExpr, Op>::get_combined_expr() const
	-> CombinedExprRef
{
	const auto& combined_ptr = std::get<CombinedExpr>(m_value);

	return CombinedExprRef{
		std::cref(*std::get<0>(combined_ptr)),
		std::cref(*std::get<1>(combined_ptr)),
		std::cref(*std::get<2>(combined_ptr)),
	};
}

template <typename SelfExpr, typename HigherExpr, typename Op>
	requires std::is_base_of_v<::toycc::Operator, Op>
BinaryExpr<SelfExpr, HigherExpr, Op>::~BinaryExpr()
{
}


}	// namespace toycc

