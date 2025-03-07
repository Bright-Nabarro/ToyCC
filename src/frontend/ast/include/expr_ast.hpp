#pragma once
#include "base_components_ast.hpp"
#include "operator_ast.hpp"

namespace toycc
{

class BaseExpr: public BaseAST
{
public:
	BaseExpr(AstKind ast_kind, std::unique_ptr<Location> location);

	[[nodiscard]] static
	auto classof(const BaseAST* ast) -> bool;
};


class LOrExpr;
using LowExpr = LOrExpr;
/**
 * Expr ::= ExprL4;
 */
class Expr: public BaseExpr
{
public:
	TOYCC_AST_FILL_CLASSOF(ast_expr)

	Expr(std::unique_ptr<Location> location, std::unique_ptr<LowExpr> uptr);
	~Expr();
	
	[[nodiscard]]
	auto get_low_expr() const -> const LowExpr&;

private:
	std::unique_ptr<LowExpr> m_value;

};


class ExprList: public BaseAST
{
public:
	TOYCC_AST_FILL_CLASSOF(ast_expr_list);
	using Vector = std::vector<std::unique_ptr<Expr>>;
	ExprList(std::unique_ptr<Location> location);
	ExprList(std::unique_ptr<Location> location,
			 std::unique_ptr<ExprList> expr_list,
			 std::unique_ptr<Expr> expr);
	
	[[nodiscard]]
	auto get_expr_list() const -> const Vector&;
	
	[[nodiscard]]
	auto begin() const -> Vector::const_iterator
	{ return m_expr_list.cbegin(); }

	[[nodiscard]]
	auto end() const -> Vector::const_iterator
	{ return m_expr_list.cend(); }

	[[nodiscard]]
	auto size() const -> std::size_t
	{ return m_expr_list.size(); }
private:
	Vector m_expr_list;
};


/**
 * ConstInitVal 	::= ConstExpr;
 */
class ConstExpr: public BaseExpr
{
public:
	TOYCC_AST_FILL_CLASSOF(ast_const_expr);
	ConstExpr(std::unique_ptr<Location> location, std::unique_ptr<Expr> expr);

	[[nodiscard]]
	auto get_expr() const -> const Expr&;

private:
	std::unique_ptr<Expr> m_expr;
};


/**
 * PrimaryExpr  ::= "(" Expr ")" | Number | Ident;
 */
class PrimaryExpr: public BaseExpr
{
public:
	TOYCC_AST_FILL_CLASSOF(ast_primary_expr);
	using ExprPtr = std::unique_ptr<Expr>;
	using NumberPtr = std::unique_ptr<Number>;
	using LValPtr = std::unique_ptr<LVal>;
	using Variant = std::variant<ExprPtr, NumberPtr, LValPtr>;

	PrimaryExpr(std::unique_ptr<Location> location, ExprPtr expr_ptr);
	PrimaryExpr(std::unique_ptr<Location> location, NumberPtr number_ptr);
	PrimaryExpr(std::unique_ptr<Location> location, LValPtr lval_ptr);

	[[nodiscard]]
	auto has_expr() const -> bool;
	[[nodiscard]]
	auto has_number() const -> bool;
	[[nodiscard]]
	auto has_ident() const -> bool;

	// 暂时搁置对于visit的实现
	// 需要详细了解std::invoke_result_t中对于lambda表达式和仿函数参数的区别
	// 并且这里的Variant存储类型为unique_ptr, 这个信息需要对用户屏蔽，
	// 让用户传入的回调函数直接处理对应类型
	
	//template <typename Func>
	//auto visit(Func&& func) const -> util::nf_visit_result_t<Func, Variant>
	//{
	//	//return std::visit(util::uptr_deref_func<Func, Variant>::func(
	//	//					  std::forward<Func>(func)),
	//	//				  m_value);
	//}
	
	[[nodiscard]]
	auto get_expr() const -> const Expr&;
	[[nodiscard]]
	auto get_lval() const -> const LVal&;
	[[nodiscard]]
	auto get_number() const -> const Number&;
	
private:
	Variant m_value;
};


class PassingParams: public BaseAST
{
public:
	TOYCC_AST_FILL_CLASSOF(ast_passing_params);
	PassingParams(std::unique_ptr<Location> location,
				  std::unique_ptr<Expr> expr,
				  std::unique_ptr<ExprList> expr_list);
	auto size() const -> std::size_t;
	auto get_expr() const -> const Expr&;
	auto get_expr_list() const -> const ExprList&;
private:
	std::unique_ptr<Expr> m_expr;
	std::unique_ptr<ExprList> m_expr_list;
};


/**
 * UnaryExpr ::= PrimaryExpr | UnaryOp UnaryExpr;
 */
class UnaryExpr: public BaseExpr
{
public:
	enum UnaryType
	{
		primary_expr,
		unary_op,
		call,
		call_with_params,
	};

	UnaryExpr(std::unique_ptr<Location> location,
			  UnaryType type,
			  std::unique_ptr<PrimaryExpr> primary_expr);
	UnaryExpr(std::unique_ptr<Location> location,
			  UnaryType type,
			  std::unique_ptr<UnaryOp> unary_op,
			  std::unique_ptr<UnaryExpr> unary_expr);
	UnaryExpr(std::unique_ptr<Location> location,
			  UnaryType type,
			  std::unique_ptr<Ident> ident);
	UnaryExpr(std::unique_ptr<Location> location,
			  UnaryType type,
			  std::unique_ptr<Ident> ident,
			  std::unique_ptr<PassingParams> passing_param);

	[[nodiscard]]
	auto get_unary_type() const -> UnaryType;
	[[nodiscard]]
	auto get_primary_expr() const -> const PrimaryExpr&;
	[[nodiscard]]
	auto get_unary_op() const -> const UnaryOp&;
	[[nodiscard]]
	auto get_unary_expr() const -> const UnaryExpr&;
	[[nodiscard]]
	auto get_ident() const -> const Ident&;
	[[nodiscard]]
	auto get_passing_params() const -> const PassingParams&;

private:
	UnaryType m_type;
	std::unique_ptr<PrimaryExpr> m_primary_expr;
	std::unique_ptr<UnaryOp> m_unary_op;
	std::unique_ptr<UnaryExpr> m_unary_expr;
	std::unique_ptr<Ident> m_ident;
	std::unique_ptr<PassingParams> m_passing_params;
};


class BinaryExprBase: public BaseExpr
{
public:
	using BaseExpr::BaseExpr;
};


template<typename SelfExpr, typename HigherExpr, typename Op>
	requires std::is_base_of_v<::toycc::Operator, Op>
class BinaryExpr: public BinaryExprBase
{
public:
	using SelfExprPtr = std::unique_ptr<SelfExpr>;
	using HigherExprPtr = std::unique_ptr<HigherExpr>;
	using OpPtr = std::unique_ptr<Op>;

	using CombinedExpr = std::tuple<
		SelfExprPtr,
		OpPtr,
		HigherExprPtr
	>;
	using CombinedExprRef = std::tuple<
		std::reference_wrapper<const SelfExpr>,
		std::reference_wrapper<const Op>,
		std::reference_wrapper<const HigherExpr>
	>;
	using Variant = std::variant<HigherExprPtr, CombinedExpr>;

	BinaryExpr(AstKind kind, std::unique_ptr<Location> location, HigherExprPtr ptr);
	BinaryExpr(AstKind kind, std::unique_ptr<Location> location, SelfExprPtr self_ptr,
			   OpPtr op_ptr, HigherExprPtr higher_ptr);
	~BinaryExpr() = 0;

	[[nodiscard]]
	auto has_higher_expr() const -> bool;
	[[nodiscard]]
	auto has_combined_expr() const -> bool;
	[[nodiscard]]
	auto get_higher_expr() const -> const HigherExpr&;
	[[nodiscard]]
	auto get_combined_expr() const -> CombinedExprRef;

private:
	Variant m_value;
};


#define DEFINE_BINARY_EXPR_CLASS(expr_kind, expr_name, operation, higher_expr) \
	class expr_name : public BinaryExpr<expr_name, higher_expr, operation>     \
	{                                                                          \
	public:                                                                    \
		TOYCC_AST_FILL_CLASSOF(expr_kind)                                      \
		expr_name(std::unique_ptr<Location> location, HigherExprPtr ptr)       \
			: BinaryExpr{expr_kind, std::move(location), std::move(ptr)}       \
		{                                                                      \
		}                                                                      \
		expr_name(std::unique_ptr<Location> location, SelfExprPtr self_ptr,    \
				  OpPtr op_ptr, HigherExprPtr higher_ptr)                      \
			: BinaryExpr{expr_kind, std::move(location), std::move(self_ptr),  \
						 std::move(op_ptr), std::move(higher_ptr)}             \
		{                                                                      \
		}                                                                      \
	};                                                                         \
	template class BinaryExpr<expr_name, higher_expr, operation>;

/// L3Expr ::= UnaryExpr | L3Expr L3Op UnaryExpr;
DEFINE_BINARY_EXPR_CLASS(ast_l3expr, L3Expr, L3Op, UnaryExpr)

/// L4Expr ::= L3Expr | L4Expr L4Op L3Expr;
DEFINE_BINARY_EXPR_CLASS(ast_l4expr, L4Expr, L4Op, L3Expr)

/**
 *	L6Expr ::= L4Expr | L6Expr L6Op L4Expr;
 */
DEFINE_BINARY_EXPR_CLASS(ast_l6expr, L6Expr, L6Op, L4Expr)

/**
 * L7Expr ::= L6Expr | L7Expr L7Op L6Expr;
 */
DEFINE_BINARY_EXPR_CLASS(ast_l7expr, L7Expr, L7Op, L6Expr);

/**
 * LAndExpr	::= L7Expr | LAndExpr LAndOp L7Expr;
 */
DEFINE_BINARY_EXPR_CLASS(ast_land_expr, LAndExpr, LAndOp, L7Expr)

/**
 * LOrExpr	::= LAndExpr | LOrExpr LOrOp LAndExpr;
 */
DEFINE_BINARY_EXPR_CLASS(ast_lor_expr, LOrExpr, LOrOp, LAndExpr)


#undef DEFINE_BINARY_EXPR_CLASS
}	//namespace toycc

