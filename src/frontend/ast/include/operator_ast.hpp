#pragma once
#include <cassert>
#include <format>
#include <spdlog/spdlog.h>
#include "base_ast.hpp"

namespace toycc
{
/**
 * @brief 所有操作符的基类
 * @note 在各个派生类的构造函数中，需要检查type是否合法
 */
class Operator: public BaseAST
{
public:

	virtual ~Operator() = 0;
	[[nodiscard]]
	static auto classof(const BaseAST* ast) -> bool;

	enum OperationType
	{
		op_add,
		op_sub,
		op_not,
		op_mul,
		op_div,
		op_mod,
		op_lt,
		op_le,
		op_gt,
		op_ge,
		op_eq,
		op_ne,
		op_land,
		op_lor,
	};
	
	Operator(AstKind ast_kind, std::unique_ptr<Location> location, OperationType type);

	auto get_type() const -> OperationType;
	auto get_type_str() const -> const char*;

protected:
	OperationType m_type;
};

#define OP_TYPE_CHECK(expr, param_type)                                        \
	do                                                                         \
	{                                                                          \
		if (expr)                                                              \
		{                                                                      \
			spdlog::error("Invalid OperationType for {} {}", get_kind_str(),   \
						  static_cast<int>(param_type));                       \
			exit(-1);                                                          \
		}                                                                      \
	} while (0)

/**
 * UnaryOp ::= "+" | "-" | "!";
 */
class UnaryOp: public Operator
{
public:
	TOYCC_AST_FILL_CLASSOF(ast_unary_op);

	UnaryOp(std::unique_ptr<Location> location, OperationType type):
		Operator { ast_unary_op, std::move(location), type}
	{
		OP_TYPE_CHECK(get_type() < op_add || get_type() > op_not, type);
	}
};


/// L3Op ::= "*" | "/" | "%"
class L3Op: public Operator
{
public:
	TOYCC_AST_FILL_CLASSOF(ast_l3op);

	L3Op(std::unique_ptr<Location> location, OperationType type):
		Operator(ast_l3op, std::move(location), type)
	{
		OP_TYPE_CHECK(get_type() < op_mul || get_type() > op_mod, type);
	}
};


/// L4Op ::= "+" | "-"
class L4Op: public Operator
{
public:
	TOYCC_AST_FILL_CLASSOF(ast_l4op);

	L4Op(std::unique_ptr<Location> location, OperationType type):
		Operator(ast_l4op, std::move(location), type)
	{
		OP_TYPE_CHECK(get_type() < op_add || get_type() > op_not, type);
	}
};


/**
 * L6Op	::= "<" | ">" | "<=" | ">="
 */
class L6Op: public Operator
{
public:
	TOYCC_AST_FILL_CLASSOF(ast_l6op);

	L6Op(std::unique_ptr<Location> location, OperationType type):
		Operator(ast_l6op, std::move(location), type)
	{
		OP_TYPE_CHECK(get_type() < op_lt || get_type() > op_ge, type);
	}
};


/**
 * L7Op	::= "==" | "!="
 */
class L7Op : public Operator
{
public:
	TOYCC_AST_FILL_CLASSOF(ast_l7op)

	L7Op(std::unique_ptr<Location> location, OperationType type)
		: Operator{ast_l7op, std::move(location), type}
	{
		OP_TYPE_CHECK(get_type() < op_eq || get_type() > op_ne, type);
	}
};


/**
 * LAndOp		::= "&&"
 */
class LAndOp: public Operator
{
public:
	TOYCC_AST_FILL_CLASSOF(ast_land_op);
	LAndOp(std::unique_ptr<Location> location, OperationType type)
		: Operator{ast_land_op, std::move(location), type}
	{
		OP_TYPE_CHECK(get_type() == op_land, type);
	}
};


/**
 * LOrOp		::= "||"
 */
class LOrOp: public Operator
{
public:
	TOYCC_AST_FILL_CLASSOF(ast_lor_op);
	LOrOp(std::unique_ptr<Location> location, OperationType type)
		: Operator{ast_lor_op, std::move(location), type}
	{
		OP_TYPE_CHECK(get_type() == op_lor, type);
	}
};

}	//namespace toycc
