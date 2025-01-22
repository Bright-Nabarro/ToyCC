#include <easylog.hpp>
#include "base_components_ast.hpp"

namespace tinyc
{

/// Number
Number::Number(std::unique_ptr<Location> location, int value)
	: BaseAST{ast_number, std::move(location)}, m_value{value}
{
}

auto Number::get_int_literal() const -> int
{ return m_value; }


/// Ident
Ident::Ident(std::unique_ptr<Location> location, std::string value)
	: BaseAST{ast_ident, std::move(location)}, m_value{std::move(value)}
{
}

auto Ident::get_value() const -> std::string_view
{ return m_value; }


/// Type
ScalarType::ScalarType(std::unique_ptr<Location> location, BuiltinTypeEnum type)
	: BaseAST { ast_scalar_type, std::move(location) }, m_type { type }
{
	if (m_type == BuiltinTypeEnum::ty_void)
		yq::error("Expected type void in ScalarType constructor");
}

auto ScalarType::get_type() const -> BuiltinTypeEnum
{
	return m_type;
}

auto ScalarType::get_type_str() const -> const char*
{
	return get_builtin_type_str(get_type());
}


BuiltinType::BuiltinType(std::unique_ptr<Location> location, BuiltinTypeEnum type)
	: BaseAST{ast_builtin_type, std::move(location)}, m_type{type}
{
}

auto BuiltinType::get_type() const -> BuiltinTypeEnum
{ return m_type; }

auto BuiltinType::get_type_str() const -> const char*
{
	return get_builtin_type_str(get_type());
}


//LVal
LVal::LVal(std::unique_ptr<Location> location, std::unique_ptr<Ident> ident):
	BaseAST { ast_lval, std::move(location) },
	m_ident { std::move(ident) }
{}

auto LVal::get_id() const -> const Ident&
{
	return *m_ident;
}

}	//namespace tinyc
