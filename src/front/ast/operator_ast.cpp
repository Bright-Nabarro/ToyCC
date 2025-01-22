#include "operator_ast.hpp"

namespace tinyc
{

/// Operation

Operator::~Operator(){}

auto Operator::classof(const BaseAST* ast) -> bool
{
	return ast->get_kind() > ast_op &&
		   ast->get_kind() < ast_op_end;
}

Operator::Operator(AstKind ast_kind, std::unique_ptr<Location> location,
					 OperationType type)
	: BaseAST{ast_kind, std::move(location)}, m_type{type}
{
	assert(ast_kind > ast_op && ast_kind < ast_op_end);
}

auto Operator::get_type() const -> OperationType
{
	return m_type;
}

auto Operator::get_type_str() const -> const char*
{
	switch(get_type())
	{
	case op_add:
		return "add";
	case op_sub:
		return "sub";
	case op_not:
		return "not";
	case op_mul:
		return "mul";
	case op_div:
		return "div";
	case op_mod:
		return "mod";
	case op_lt:
		return "lt";
	case op_le:
		return "le";
	case op_gt:
		return "gt";
	case op_ge:
		return "ge";
	case op_eq:
		return "eq";
	case op_ne:
		return "ne";
	case op_land:
		return "land";
	case op_lor:
		return "lor";
	default:
		return "unkown";
	};
}

}	//namespace tinyc
