%skeleton "lalr1.cc"	// use LALR-1, C++ language
%require "3.8.2"
%header
%define api.token.raw  			// 禁止在文法推导中使用字符字面量 
%define api.token.constructor 	// 将symbol作为一个类型
%define api.value.type variant 	// 使用variant代替union

%code requires {
#include <memory>
#include <string>
#include <string_view>
#include <iostream>
#include "ast.hpp"
#include "llvm_location.hpp"
//前向声明
namespace toycc { class Driver; }
}

%param { toycc::Driver& driver }

%locations	//生成location定位
%define api.location.type { toycc::LLVMLocation }	//使用自定义location类型
//%define api.filename.type { std::string_view }
%define parse.trace
%define parse.error detailed
%define parse.lac full	//启用lac前瞻

%code {
#include <memory>
#include <cassert>
#include <string>
#include <llvm/Support/Casting.h>
#include "llvm_location.hpp"
#include "driver.hpp"
#define assert_same_ptr(Type, ptr) \
	static_assert(std::is_same_v<Type, typename std::decay_t<decltype(ptr)>::element_type>)

#define CONSTRUCT_LOCATION(arg) \
	std::make_unique<toycc::LLVMLocation>(arg)
}

%token <std::string> IDENT
%token <int> INT_LITERAL
//关键字
%token KW_RETURN
%token KW_SINT KW_UINT KW_VOID 
%token KW_CONST KW_EVAL
%token KW_IF KW_ELSE
// 字面量标识分隔符
%token DELIM_LPAREN		"("
%token DELIM_RPAREN		")"
%token DELIM_LBRACE		"{"
%token DELIM_RBRACE		"}"
%token DELIM_COMMA 		","
%token DELIM_SEMICOLON	";"
// 操作符
%token OP_ADD	"+"
%token OP_SUB	"-"
%token OP_NOT	"!"
%token OP_MUL	"*"
%token OP_DIV	"/"
%token OP_MOD	"%"
%token OP_LT	"<"
%token OP_LE	"<="
%token OP_GT	">"
%token OP_GE	">="
%token OP_EQ	"=="
%token OP_NE	"!="
%token OP_LAND	"&&"
%token OP_LOR	"||"
%token OP_ASSIGN "="

%nterm <std::unique_ptr<toycc::CompUnit>>		CompUnit
//basic
%nterm <std::unique_ptr<toycc::Number>>			Number
%nterm <std::unique_ptr<toycc::Ident>>			Ident
%nterm <std::unique_ptr<toycc::LVal>>			LVal
//语句
%nterm <std::unique_ptr<toycc::Stmt>>			Stmt
%nterm <std::unique_ptr<toycc::Decl>>			Decl
%nterm <std::unique_ptr<toycc::Block>>			Block
%nterm <std::unique_ptr<toycc::BlockItemList>>	BlockItemList
%nterm <std::unique_ptr<toycc::BlockItem>>		BlockItem
//变量声明，使用
%nterm <std::unique_ptr<toycc::ConstDecl>>		ConstDecl
%nterm <std::unique_ptr<toycc::ConstDef>> 		ConstDef
%nterm <std::unique_ptr<toycc::ConstDefList>> 	ConstDefList
%nterm <std::unique_ptr<toycc::ConstInitVal>>	ConstInitVal
%nterm <std::unique_ptr<toycc::ConstExpr>>		ConstExpr
%nterm <std::unique_ptr<toycc::VarDecl>>		VarDecl
%nterm <std::unique_ptr<toycc::VarDef>>			VarDef
%nterm <std::unique_ptr<toycc::VarDefList>>		VarDefList
%nterm <std::unique_ptr<toycc::InitVal>>		InitVal
//type
%nterm <std::unique_ptr<toycc::ScalarType>>		ScalarType
%nterm <std::unique_ptr<toycc::BuiltinType>>	BuiltinType

%nterm <std::unique_ptr<toycc::Param>>			Param
%nterm <std::unique_ptr<toycc::ParamList>>		ParamList
%nterm <std::unique_ptr<toycc::FuncDef>>		FuncDef
// expr
%nterm <std::unique_ptr<toycc::Expr>>			Expr
%nterm <std::unique_ptr<toycc::UnaryExpr>>		UnaryExpr
%nterm <std::unique_ptr<toycc::PrimaryExpr>>	PrimaryExpr
%nterm <std::unique_ptr<toycc::L3Expr>> 		L3Expr
%nterm <std::unique_ptr<toycc::L4Expr>>			L4Expr
%nterm <std::unique_ptr<toycc::L6Expr>>			L6Expr
%nterm <std::unique_ptr<toycc::L7Expr>>			L7Expr
%nterm <std::unique_ptr<toycc::LAndExpr>>		LAndExpr
%nterm <std::unique_ptr<toycc::LOrExpr>>		LOrExpr
// operator
%nterm <std::unique_ptr<toycc::UnaryOp>>		UnaryOp
%nterm <std::unique_ptr<toycc::L3Op>> 			L3Op
%nterm <std::unique_ptr<toycc::L4Op>>			L4Op
%nterm <std::unique_ptr<toycc::L6Op>>			L6Op
%nterm <std::unique_ptr<toycc::L7Op>>			L7Op
%nterm <std::unique_ptr<toycc::LAndOp>>			LAndOp
%nterm <std::unique_ptr<toycc::LOrOp>>			LOrOp


%%

%start CompUnit;

CompUnit:
	FuncDef 
	{
		//llvm::isa足够智能，能够区分裸指针和智能指针的情况
		assert_same_ptr(toycc::FuncDef, $1);
		std::unique_ptr<toycc::Location> location =
			CONSTRUCT_LOCATION(@$);
		auto comp_unit_ptr =
			std::make_unique<toycc::CompUnit>(std::move(location), std::move($1));
		driver.set_ast(std::move(comp_unit_ptr));
	};

FuncDef :
//   1	   2 	3      4   	  5   6
	BuiltinType Ident "(" ParamList ")" Block
	{
		assert_same_ptr(toycc::BuiltinType,$1);
		assert_same_ptr(toycc::Ident, $2);
		assert_same_ptr(toycc::ParamList, $4);
		assert_same_ptr(toycc::Block, $6);

		auto funcdef_ptr = std::make_unique<toycc::FuncDef>(
			CONSTRUCT_LOCATION(@$),
			std::move($1), std::move($2), std::move($4), std::move($6)
		);

		$$ = std::move(funcdef_ptr);
	};

ParamList :
	/* empty */
	{
		$$ = std::make_unique<toycc::ParamList>(CONSTRUCT_LOCATION(@$));
	}
	| Param
	{
		assert_same_ptr(toycc::Param, $1);
		auto param_list_ptr = std::make_unique<toycc::ParamList>(CONSTRUCT_LOCATION(@$));
		param_list_ptr->add_param(std::move($1));
		$$ = std::move(param_list_ptr);
	}
	| ParamList "," Param
	{
		assert_same_ptr(toycc::ParamList, $1);
		assert_same_ptr(toycc::Param, $3);

		auto param_list_ptr = std::move($1);
		param_list_ptr->add_param(std::move($3));
		$$ = std::move(param_list_ptr);
	}

Param :
	ScalarType Ident
	{
		assert_same_ptr(toycc::ScalarType, $1);
		assert_same_ptr(toycc::Ident, $2);
		auto param_ptr = std::make_unique<toycc::Param>(CONSTRUCT_LOCATION(@$), std::move($1), std::move($2));
		$$ = std::move(param_ptr);
	}

ScalarType        
	: KW_SINT {
		$$ = std::make_unique<toycc::ScalarType>(CONSTRUCT_LOCATION(@$), toycc::BuiltinTypeEnum::ty_signed_int);
	}
	| KW_UINT {
		$$ = std::make_unique<toycc::ScalarType>(CONSTRUCT_LOCATION(@$), toycc::BuiltinTypeEnum::ty_unsigned_int);
	};

BuiltinType
	: ScalarType {
		assert_same_ptr(toycc::ScalarType, $1);
		// 直接构造，与文法原本语义不同
		$$ = std::make_unique<toycc::BuiltinType>(CONSTRUCT_LOCATION(@$), ($1)->get_type());
	}
	| KW_VOID {
		$$ = std::make_unique<toycc::BuiltinType>(CONSTRUCT_LOCATION(@$),
			toycc::BuiltinTypeEnum::ty_void);
	};

Decl
	: ConstDecl {
		assert_same_ptr(toycc::ConstDecl, $1);
		$$ = std::make_unique<toycc::Decl>(CONSTRUCT_LOCATION(@$), std::move($1));
	}
	| VarDecl {
		$$ = std::make_unique<toycc::Decl>(CONSTRUCT_LOCATION(@$), std::move($1));
	};

ConstDecl
	: KW_EVAL ScalarType ConstDef ConstDefList ";" {
	// 1 		2 			3 			4		 
		assert_same_ptr(toycc::ScalarType, $2);
		assert_same_ptr(toycc::ConstDef, $3);
		assert_same_ptr(toycc::ConstDefList, $4);
		$$ = std::make_unique<toycc::ConstDecl>(CONSTRUCT_LOCATION(@$),
			std::move($2), std::move($3), std::move($4));
	};

ConstDefList
	: /*empty*/ {
		$$ = std::make_unique<toycc::ConstDefList>(CONSTRUCT_LOCATION(@$));
	}
	// 1			2	3
	| ConstDefList "," ConstDef {
		assert_same_ptr(toycc::ConstDefList, $1);
		assert_same_ptr(toycc::ConstDef, $3);
		$$ = std::make_unique<toycc::ConstDefList>(CONSTRUCT_LOCATION(@$),
			std::move($1), std::move($3));
	};

ConstDef
	: Ident "=" ConstInitVal {
	// 1	 2		3
		assert_same_ptr(toycc::Ident, $1);
		assert_same_ptr(toycc::ConstInitVal, $3);
		$$ = std::make_unique<toycc::ConstDef>(CONSTRUCT_LOCATION(@$),
			std::move($1), std::move($3));
	};

ConstInitVal 	
	: ConstExpr {
		assert_same_ptr(toycc::ConstExpr, $1);
		$$ = std::make_unique<toycc::ConstInitVal>(CONSTRUCT_LOCATION(@$),
			std::move($1));
	}; 

VarDecl 
	: ScalarType VarDef VarDefList ";" {
		$$ = std::make_unique<toycc::VarDecl>(CONSTRUCT_LOCATION(@$),
			std::move($1), std::move($2), std::move($3)
		);
	};

VarDef 	
	: Ident {
		$$ = std::make_unique<toycc::VarDef>(CONSTRUCT_LOCATION(@$),
			std::move($1)
		);
	}
	| Ident "=" InitVal {
		$$ = std::make_unique<toycc::VarDef>(CONSTRUCT_LOCATION(@$),
			std::move($1), std::move($3)
		);
	};

VarDefList		
	: /* empty */ {
		$$ = std::make_unique<toycc::VarDefList>(CONSTRUCT_LOCATION(@$));
	}
	| VarDefList "," VarDef {
		$$ = std::make_unique<toycc::VarDefList>(CONSTRUCT_LOCATION(@$),
			std::move($1), std::move($3));
	};

InitVal 		
	: Expr {
		$$ = std::make_unique<toycc::InitVal>(CONSTRUCT_LOCATION(@$),
			std::move($1));
	};

ConstExpr
	: Expr {
		assert_same_ptr(toycc::Expr, $1);
		$$ = std::make_unique<toycc::ConstExpr>(CONSTRUCT_LOCATION(@$),
			std::move($1));
	};

Block
	: "{" BlockItemList "}"{
		assert_same_ptr(toycc::BlockItemList, $2);
		$$ = std::make_unique<toycc::Block>(CONSTRUCT_LOCATION(@$), std::move($2));
	};

BlockItemList
	: /* empty */ {
		$$ = std::make_unique<toycc::BlockItemList>(CONSTRUCT_LOCATION(@$));
	}
	| BlockItem BlockItemList {
		assert_same_ptr(toycc::BlockItem, $1);
		assert_same_ptr(toycc::BlockItemList, $2);
		$$ = std::make_unique<toycc::BlockItemList>(CONSTRUCT_LOCATION(@$), 
			std::move($1), std::move($2));
	};

BlockItem
	: Decl {
		assert_same_ptr(toycc::Decl, $1);
		$$ = std::make_unique<toycc::BlockItem>(CONSTRUCT_LOCATION(@$), 
			std::move($1));
	}
	| Stmt {
		assert_same_ptr(toycc::Stmt, $1);
		$$ = std::make_unique<toycc::BlockItem>(CONSTRUCT_LOCATION(@$), 
			std::move($1));
	};

LVal
	: Ident {
		assert_same_ptr(toycc::Ident, $1);
		$$ = std::make_unique<toycc::LVal>(CONSTRUCT_LOCATION(@$), std::move($1));
	};

Stmt
	: LVal "=" Expr DELIM_SEMICOLON {
		$$ = std::make_unique<toycc::Stmt>(CONSTRUCT_LOCATION(@$),
			toycc::Stmt::assign, std::move($1), std::move($3));
	}
	| Expr DELIM_SEMICOLON {
		$$ = std::make_unique<toycc::Stmt>(CONSTRUCT_LOCATION(@$),
			toycc::Stmt::expression, std::move($1));
	}
	| DELIM_SEMICOLON {
		$$ = std::make_unique<toycc::Stmt>(CONSTRUCT_LOCATION(@$),
			toycc::Stmt::expression);
	}
	| KW_RETURN Expr DELIM_SEMICOLON {
		assert_same_ptr(toycc::Expr, $2);
		auto stmt_ptr = std::make_unique<toycc::Stmt>(CONSTRUCT_LOCATION(@$),
			toycc::Stmt::func_return, std::move($2));
		$$ = std::move(stmt_ptr);
	}
	| KW_RETURN DELIM_SEMICOLON {
		$$ = std::make_unique<toycc::Stmt>(CONSTRUCT_LOCATION(@$),
			toycc::Stmt::func_return);
	}
	| Block {
		$$ = std::make_unique<toycc::Stmt>(CONSTRUCT_LOCATION(@$),
			toycc::Stmt::block, std::move($1));
	}
	| KW_IF "(" Expr ")" Stmt {
		$$ = std::make_unique<toycc::Stmt>(CONSTRUCT_LOCATION(@$),
			toycc::Stmt::if_stmt, std::move($3), std::move($5));
	}
	// 1     2   3    4   5     6      7
	| KW_IF "(" Expr ")" Stmt KW_ELSE Stmt {
		$$ = std::make_unique<toycc::Stmt>(CONSTRUCT_LOCATION(@$),
			toycc::Stmt::if_stmt, std::move($3), std::move($5), std::move($7));
	};

Expr
	: LOrExpr {
		assert_same_ptr(toycc::LOrExpr, $1);
		$$ = std::make_unique<toycc::Expr>(CONSTRUCT_LOCATION(@$), std::move($1));
	};

PrimaryExpr
	: "(" Expr ")" {
		assert_same_ptr(toycc::Expr, $2);
		$$ = std::make_unique<toycc::PrimaryExpr>(CONSTRUCT_LOCATION(@$), std::move($2));
	}
	| Number {
		assert_same_ptr(toycc::Number, $1);
		$$ = std::make_unique<toycc::PrimaryExpr>(CONSTRUCT_LOCATION(@$), std::move($1));
	}
	| LVal {
		assert_same_ptr(toycc::LVal, $1);
		$$ = std::make_unique<toycc::PrimaryExpr>(CONSTRUCT_LOCATION(@$), std::move($1));
	};


UnaryExpr
	: PrimaryExpr {
		assert_same_ptr(toycc::PrimaryExpr, $1);
		$$ = std::make_unique<toycc::UnaryExpr>(CONSTRUCT_LOCATION(@$), std::move($1));
	}
	| UnaryOp UnaryExpr {
		assert_same_ptr(toycc::UnaryOp, $1);
		assert_same_ptr(toycc::UnaryExpr, $2);
		$$ = std::make_unique<toycc::UnaryExpr>(CONSTRUCT_LOCATION(@$), std::move($1), std::move($2));
	};

UnaryOp
	: "+" {
		$$ = std::make_unique<toycc::UnaryOp>(CONSTRUCT_LOCATION(@$), toycc::UnaryOp::op_add);
	}
	| "-" {
		$$ = std::make_unique<toycc::UnaryOp>(CONSTRUCT_LOCATION(@$), toycc::UnaryOp::op_sub);
	} 
	| "!" {
		$$ = std::make_unique<toycc::UnaryOp>(CONSTRUCT_LOCATION(@$), toycc::UnaryOp::op_not);
	};

L3Expr
	: UnaryExpr {
		assert_same_ptr(toycc::UnaryExpr, $1);
		$$ = std::make_unique<toycc::L3Expr>(CONSTRUCT_LOCATION(@$), std::move($1));
	}
	| L3Expr L3Op UnaryExpr {
		assert_same_ptr(toycc::L3Expr, $1);
		assert_same_ptr(toycc::L3Op, $2);
		assert_same_ptr(toycc::UnaryExpr, $3);
		$$ = std::make_unique<toycc::L3Expr>(CONSTRUCT_LOCATION(@$), std::move($1), std::move($2), std::move($3));
	};

L3Op
	: "*"  {
		$$ = std::make_unique<toycc::L3Op>(CONSTRUCT_LOCATION(@$), toycc::Operator::op_mul);
	}
	| "/"  {
		$$ = std::make_unique<toycc::L3Op>(CONSTRUCT_LOCATION(@$), toycc::Operator::op_div);
	}
	| "%" {
		$$ = std::make_unique<toycc::L3Op>(CONSTRUCT_LOCATION(@$), toycc::Operator::op_mod);
	};

L4Expr
	: L3Expr {
		assert_same_ptr(toycc::L3Expr, $1);
		$$ = std::make_unique<toycc::L4Expr>(CONSTRUCT_LOCATION(@$), std::move($1));
	}
	| L4Expr L4Op L3Expr {
		assert_same_ptr(toycc::L4Expr, $1);
		assert_same_ptr(toycc::L4Op, $2);
		assert_same_ptr(toycc::L3Expr, $3);
		$$ = std::make_unique<toycc::L4Expr>(
			CONSTRUCT_LOCATION(@$),
			std::move($1),
			std::move($2),
			std::move($3)
		);
	};

L4Op
	: "+" {
		$$ = std::make_unique<toycc::L4Op>(
			CONSTRUCT_LOCATION(@$),
			toycc::Operator::op_add
		);
	}
	| "-" {
		$$ = std::make_unique<toycc::L4Op>(CONSTRUCT_LOCATION(@$),
			toycc::Operator::op_sub);
	};

L6Expr
	: L4Expr {
		assert_same_ptr(toycc::L4Expr, $1);
		$$ = std::make_unique<toycc::L6Expr>(
			CONSTRUCT_LOCATION(@$),
			std::move($1)
		);
	}
	| L6Expr L6Op L4Expr {
		assert_same_ptr(toycc::L6Expr, $1);
		assert_same_ptr(toycc::L6Op, $2);
		assert_same_ptr(toycc::L4Expr, $3);
		$$ = std::make_unique<toycc::L6Expr>(CONSTRUCT_LOCATION(@$), std::move($1), std::move($2), std::move($3));
	};

L6Op
	: "<" {
		$$ = std::make_unique<toycc::L6Op>(CONSTRUCT_LOCATION(@$), toycc::Operator::op_lt);
	}
	| ">" {
		$$ = std::make_unique<toycc::L6Op>(CONSTRUCT_LOCATION(@$), toycc::Operator::op_gt);
	}
	| "<=" {
		$$ = std::make_unique<toycc::L6Op>(CONSTRUCT_LOCATION(@$), toycc::Operator::op_le);
	}
	| ">=" {
		$$ = std::make_unique<toycc::L6Op>(CONSTRUCT_LOCATION(@$), toycc::Operator::op_ge);
	};

L7Expr      
	: L6Expr {
		assert_same_ptr(toycc::L6Expr, $1);
		$$ = std::make_unique<toycc::L7Expr>(CONSTRUCT_LOCATION(@$), std::move($1));
	}
	| L7Expr L7Op L6Expr {
		assert_same_ptr(toycc::L7Expr, $1);
		assert_same_ptr(toycc::L7Op, $2);
		assert_same_ptr(toycc::L6Expr, $3);
		$$ = std::make_unique<toycc::L7Expr>(CONSTRUCT_LOCATION(@$), std::move($1), std::move($2), std::move($3));
	};

L7Op		
	: "==" {
		$$ = std::make_unique<toycc::L7Op>(CONSTRUCT_LOCATION(@$), toycc::Operator::op_eq);
	}
	| "!=" {
		$$ = std::make_unique<toycc::L7Op>(CONSTRUCT_LOCATION(@$), toycc::Operator::op_ne);
	};

LAndExpr	
	: L7Expr {
		assert_same_ptr(toycc::L7Expr, $1);
		$$ = std::make_unique<toycc::LAndExpr>(CONSTRUCT_LOCATION(@$), std::move($1));
	}
	| LAndExpr LAndOp L7Expr{
		assert_same_ptr(toycc::LAndExpr, $1);
		assert_same_ptr(toycc::LAndOp, $2);
		assert_same_ptr(toycc::L7Expr, $3);
		$$ = std::make_unique<toycc::LAndExpr>(CONSTRUCT_LOCATION(@$), std::move($1), std::move($2), std::move($3));
	};

LAndOp		
	: "&&" {
		$$ = std::make_unique<toycc::LAndOp>(CONSTRUCT_LOCATION(@$), toycc::Operator::op_land);
	}

LOrExpr
	: LAndExpr {
		assert_same_ptr(toycc::LAndExpr, $1);
		$$ = std::make_unique<toycc::LOrExpr>(CONSTRUCT_LOCATION(@$), std::move($1));
	}
	| LOrExpr LOrOp LAndExpr {
		assert_same_ptr(toycc::LOrExpr, $1);
		assert_same_ptr(toycc::LOrOp, $2);
		assert_same_ptr(toycc::LAndExpr, $3);
		$$ = std::make_unique<toycc::LOrExpr>(CONSTRUCT_LOCATION(@$), std::move($1), std::move($2), std::move($3));
	};

LOrOp	
	: "||" {
		$$ = std::make_unique<toycc::LOrOp>(CONSTRUCT_LOCATION(@$), toycc::Operator::op_lor);
	};

Number
	: INT_LITERAL{
		$$ = std::make_unique<toycc::Number>(CONSTRUCT_LOCATION(@$), $1);
	};

Ident
	: IDENT{
		$$ = std::make_unique<toycc::Ident>(CONSTRUCT_LOCATION(@$), $1);
	};

%%

namespace yy
{

void parser::error(const location_type& loc, const std::string& m)
{
	loc.report(toycc::Location::dk_error, m);
}

}	//namespace yy

