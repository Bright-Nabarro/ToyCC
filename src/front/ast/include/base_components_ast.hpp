#pragma once
#include "base_ast.hpp"

namespace tinyc
{

/**
 * 存储INT_LITERAL
 * 对应文法 Number ::= INT_LITERAL;
 **/
class Number: public BaseAST
{
public:
	Number(std::unique_ptr<Location> location, int value);

	[[nodiscard]]
	auto get_int_literal() const -> int;
	
	TINYC_AST_FILL_CLASSOF(ast_number)

private:
	/// INT_LITERAL
	int m_value;
};


/**
 * 对应文法 Ident ::= [a-zA-Z_][0-9a-zA-Z_]*;
 */
class Ident: public BaseAST
{
public:
	Ident(std::unique_ptr<Location> location, std::string value);

	[[nodiscard]]
	auto get_value() const -> std::string_view;

	TINYC_AST_FILL_CLASSOF(ast_ident)
private:
	std::string m_value;
};


enum class BuiltinTypeEnum
{
	ty_void,
	ty_signed_int,
	ty_unsigned_int,
};

[[nodiscard]] constexpr
auto get_builtin_type_str(BuiltinTypeEnum type) -> const char*
{
	switch(type)
	{
	case BuiltinTypeEnum::ty_void:
		return "void";
	case BuiltinTypeEnum::ty_signed_int:
		return "signed_int";
	case BuiltinTypeEnum::ty_unsigned_int:
		return "unsigned_int";
	default:
		return "unkown";
	}
}


/**
 * ScalarType		::= SINT | UINT 	#在lexer.ll中定义其正则表达式
 */
class ScalarType: public BaseAST
{
public:
	TINYC_AST_FILL_CLASSOF(ast_scalar_type)
	ScalarType(std::unique_ptr<Location> location, BuiltinTypeEnum type);

	[[nodiscard]]
	auto get_type() const -> BuiltinTypeEnum;
	[[nodiscard]]
	auto get_type_str() const -> const char*;
private:
	BuiltinTypeEnum m_type;

};


/**
 * BuiltinType		::= ScalarType | "void" ;
 * @note 实现与文法声明不同
 */
class BuiltinType : public BaseAST
{
public:
	TINYC_AST_FILL_CLASSOF(ast_builtin_type)
	BuiltinType(std::unique_ptr<Location> location, BuiltinTypeEnum type);

	[[nodiscard]]
	auto get_type() const -> BuiltinTypeEnum;
	[[nodiscard]]
	auto get_type_str() const -> const char*;

private:
	BuiltinTypeEnum m_type;
};


/**
 *LVal		::= Ident;
 */
class LVal: public BaseAST
{
public:
	TINYC_AST_FILL_CLASSOF(ast_lval);

	LVal(std::unique_ptr<Location> location, std::unique_ptr<Ident> ident);
	[[nodiscard]]
	auto get_id() const -> const Ident&;

private:
	std::unique_ptr<Ident> m_ident;
};

}	//namespace tinyc

