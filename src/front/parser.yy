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
namespace tinyc { class Driver; }
}

%param { tinyc::Driver& driver }

%locations	//生成location定位
%define api.location.type { tinyc::LLVMLocation }	//使用自定义location类型
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
	std::make_unique<tinyc::LLVMLocation>(arg)
}

%token <std::string> IDENT
%token <int> INT_LITERAL
//关键字
%token KW_RETURN
%token KW_SINT KW_UINT KW_VOID 
%token KW_CONST
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

%nterm <std::unique_ptr<tinyc::CompUnit>>		CompUnit
//basic
%nterm <std::unique_ptr<tinyc::Number>>			Number
%nterm <std::unique_ptr<tinyc::Ident>>			Ident
%nterm <std::unique_ptr<tinyc::LVal>>			LVal

%nterm <std::unique_ptr<tinyc::Stmt>>			Stmt
%nterm <std::unique_ptr<tinyc::Decl>>			Decl
%nterm <std::unique_ptr<tinyc::Block>>			Block
%nterm <std::unique_ptr<tinyc::BlockItemList>>	BlockItemList
%nterm <std::unique_ptr<tinyc::BlockItem>>		BlockItem

%nterm <std::unique_ptr<tinyc::ConstDecl>>		ConstDecl
%nterm <std::unique_ptr<tinyc::ConstDef>> 		ConstDef
%nterm <std::unique_ptr<tinyc::ConstDefList>> 	ConstDefList
%nterm <std::unique_ptr<tinyc::ConstInitVal>>	ConstInitVal
%nterm <std::unique_ptr<tinyc::ConstExpr>>		ConstExpr
//type
%nterm <std::unique_ptr<tinyc::ScalarType>>		ScalarType
%nterm <std::unique_ptr<tinyc::BuiltinType>>	BuiltinType

%nterm <std::unique_ptr<tinyc::Param>>			Param
%nterm <std::unique_ptr<tinyc::ParamList>>		ParamList
%nterm <std::unique_ptr<tinyc::FuncDef>>		FuncDef
// expr
%nterm <std::unique_ptr<tinyc::Expr>>			Expr
%nterm <std::unique_ptr<tinyc::UnaryExpr>>		UnaryExpr
%nterm <std::unique_ptr<tinyc::PrimaryExpr>>	PrimaryExpr
%nterm <std::unique_ptr<tinyc::L3Expr>> 		L3Expr
%nterm <std::unique_ptr<tinyc::L4Expr>>			L4Expr
%nterm <std::unique_ptr<tinyc::L6Expr>>			L6Expr
%nterm <std::unique_ptr<tinyc::L7Expr>>			L7Expr
%nterm <std::unique_ptr<tinyc::LAndExpr>>		LAndExpr
%nterm <std::unique_ptr<tinyc::LOrExpr>>		LOrExpr
// operator
%nterm <std::unique_ptr<tinyc::UnaryOp>>		UnaryOp
%nterm <std::unique_ptr<tinyc::L3Op>> 			L3Op
%nterm <std::unique_ptr<tinyc::L4Op>>			L4Op
%nterm <std::unique_ptr<tinyc::L6Op>>			L6Op
%nterm <std::unique_ptr<tinyc::L7Op>>			L7Op
%nterm <std::unique_ptr<tinyc::LAndOp>>			LAndOp
%nterm <std::unique_ptr<tinyc::LOrOp>>			LOrOp


%%

%start CompUnit;

CompUnit:
	FuncDef 
	{
		//llvm::isa足够智能，能够区分裸指针和智能指针的情况
		assert_same_ptr(tinyc::FuncDef, $1);
		std::unique_ptr<tinyc::Location> location =
			CONSTRUCT_LOCATION(@$);
		auto comp_unit_ptr =
			std::make_unique<tinyc::CompUnit>(std::move(location), std::move($1));
		driver.set_ast(std::move(comp_unit_ptr));
	};

FuncDef :
//   1	   2 	3      4   	  5   6
	BuiltinType Ident "(" ParamList ")" Block
	{
		assert_same_ptr(tinyc::BuiltinType,$1);
		assert_same_ptr(tinyc::Ident, $2);
		assert_same_ptr(tinyc::ParamList, $4);
		assert_same_ptr(tinyc::Block, $6);

		auto funcdef_ptr = std::make_unique<tinyc::FuncDef>(
			CONSTRUCT_LOCATION(@$),
			std::move($1), std::move($2), std::move($4), std::move($6)
		);

		$$ = std::move(funcdef_ptr);
	};

ParamList :
	/* empty */
	{
		$$ = std::make_unique<tinyc::ParamList>(CONSTRUCT_LOCATION(@$));
	}
	| Param
	{
		assert_same_ptr(tinyc::Param, $1);
		auto param_list_ptr = std::make_unique<tinyc::ParamList>(CONSTRUCT_LOCATION(@$));
		param_list_ptr->add_param(std::move($1));
		$$ = std::move(param_list_ptr);
	}
	| ParamList "," Param
	{
		assert_same_ptr(tinyc::ParamList, $1);
		assert_same_ptr(tinyc::Param, $3);

		auto param_list_ptr = std::move($1);
		param_list_ptr->add_param(std::move($3));
		$$ = std::move(param_list_ptr);
	}

Param :
	ScalarType Ident
	{
		assert_same_ptr(tinyc::ScalarType, $1);
		assert_same_ptr(tinyc::Ident, $2);
		auto param_ptr = std::make_unique<tinyc::Param>(CONSTRUCT_LOCATION(@$), std::move($1), std::move($2));
		$$ = std::move(param_ptr);
	}

ScalarType        
	: KW_SINT {
		$$ = std::make_unique<tinyc::ScalarType>(CONSTRUCT_LOCATION(@$), tinyc::BuiltinTypeEnum::ty_signed_int);
	}
	| KW_UINT {
		$$ = std::make_unique<tinyc::ScalarType>(CONSTRUCT_LOCATION(@$), tinyc::BuiltinTypeEnum::ty_unsigned_int);
	};

BuiltinType
	: ScalarType {
		assert_same_ptr(tinyc::ScalarType, $1);
		// 直接构造，与文法原本语义不同
		$$ = std::make_unique<tinyc::BuiltinType>(CONSTRUCT_LOCATION(@$), ($1)->get_type());
	}
	| KW_VOID {
		$$ = std::make_unique<tinyc::BuiltinType>(CONSTRUCT_LOCATION(@$),
			tinyc::BuiltinTypeEnum::ty_void);
	};

Decl
	: ConstDecl {
		assert_same_ptr(tinyc::ConstDecl, $1);
		$$ = std::make_unique<tinyc::Decl>(CONSTRUCT_LOCATION(@$), std::move($1));
	};

ConstDecl
	: KW_CONST ScalarType ConstDef ConstDefList ";" {
	// 1 		2 			3 			4		 
		assert_same_ptr(tinyc::ScalarType, $2);
		assert_same_ptr(tinyc::ConstDef, $3);
		assert_same_ptr(tinyc::ConstDefList, $4);
		$$ = std::make_unique<tinyc::ConstDecl>(CONSTRUCT_LOCATION(@$),
			std::move($2), std::move($3), std::move($4));
	};

ConstDefList
	: /*empty*/ {
		$$ = std::make_unique<tinyc::ConstDefList>(CONSTRUCT_LOCATION(@$));
	}
	// 1			2	3
	| ConstDefList "," ConstDef {
		assert_same_ptr(tinyc::ConstDefList, $1);
		assert_same_ptr(tinyc::ConstDef, $3);
		$$ = std::make_unique<tinyc::ConstDefList>(CONSTRUCT_LOCATION(@$),
			std::move($1), std::move($3));
	};

ConstDef
	: Ident "=" ConstInitVal {
	// 1	 2		3
		assert_same_ptr(tinyc::Ident, $1);
		assert_same_ptr(tinyc::ConstInitVal, $3);
		$$ = std::make_unique<tinyc::ConstDef>(CONSTRUCT_LOCATION(@$),
			std::move($1), std::move($3));
	};

ConstInitVal 	
	: ConstExpr {
		assert_same_ptr(tinyc::ConstExpr, $1);
		$$ = std::make_unique<tinyc::ConstInitVal>(CONSTRUCT_LOCATION(@$),
			std::move($1));
	}; 

ConstExpr
	: Expr {
		assert_same_ptr(tinyc::Expr, $1);
		$$ = std::make_unique<tinyc::ConstExpr>(CONSTRUCT_LOCATION(@$),
			std::move($1));
	};

Block
	: "{" BlockItemList "}"{
		assert_same_ptr(tinyc::BlockItemList, $2);
		$$ = std::make_unique<tinyc::Block>(CONSTRUCT_LOCATION(@$), std::move($2));
	};

BlockItemList
	: /* empty */ {
		$$ = std::make_unique<tinyc::BlockItemList>(CONSTRUCT_LOCATION(@$));
	}
	| BlockItem BlockItemList {
		assert_same_ptr(tinyc::BlockItem, $1);
		assert_same_ptr(tinyc::BlockItemList, $2);
		$$ = std::make_unique<tinyc::BlockItemList>(CONSTRUCT_LOCATION(@$), 
			std::move($1), std::move($2));
	};

BlockItem
	: Decl {
		assert_same_ptr(tinyc::Decl, $1);
		$$ = std::make_unique<tinyc::BlockItem>(CONSTRUCT_LOCATION(@$), 
			std::move($1));
	}
	| Stmt {
		assert_same_ptr(tinyc::Stmt, $1);
		$$ = std::make_unique<tinyc::BlockItem>(CONSTRUCT_LOCATION(@$), 
			std::move($1));
	};

LVal
	: Ident {
		assert_same_ptr(tinyc::Ident, $1);
		$$ = std::make_unique<tinyc::LVal>(CONSTRUCT_LOCATION(@$), std::move($1));
	};

Stmt
	: KW_RETURN Expr ";" {
		assert_same_ptr(tinyc::Expr, $2);
		auto stmt_ptr = std::make_unique<tinyc::Stmt>(CONSTRUCT_LOCATION(@$), std::move($2));
		$$ = std::move(stmt_ptr);
	};

Expr
	: LOrExpr {
		assert_same_ptr(tinyc::LOrExpr, $1);
		$$ = std::make_unique<tinyc::Expr>(CONSTRUCT_LOCATION(@$), std::move($1));
	};

PrimaryExpr
	: "(" Expr ")" {
		assert_same_ptr(tinyc::Expr, $2);
		$$ = std::make_unique<tinyc::PrimaryExpr>(CONSTRUCT_LOCATION(@$), std::move($2));
	}
	| Number {
		assert_same_ptr(tinyc::Number, $1);
		$$ = std::make_unique<tinyc::PrimaryExpr>(CONSTRUCT_LOCATION(@$), std::move($1));
	}
	| LVal {
		assert_same_ptr(tinyc::LVal, $1);
		$$ = std::make_unique<tinyc::PrimaryExpr>(CONSTRUCT_LOCATION(@$), std::move($1));
	};


UnaryExpr
	: PrimaryExpr {
		assert_same_ptr(tinyc::PrimaryExpr, $1);
		$$ = std::make_unique<tinyc::UnaryExpr>(CONSTRUCT_LOCATION(@$), std::move($1));
	}
	| UnaryOp UnaryExpr {
		assert_same_ptr(tinyc::UnaryOp, $1);
		assert_same_ptr(tinyc::UnaryExpr, $2);
		$$ = std::make_unique<tinyc::UnaryExpr>(CONSTRUCT_LOCATION(@$), std::move($1), std::move($2));
	};

UnaryOp
	: "+" {
		$$ = std::make_unique<tinyc::UnaryOp>(CONSTRUCT_LOCATION(@$), tinyc::UnaryOp::op_add);
	}
	| "-" {
		$$ = std::make_unique<tinyc::UnaryOp>(CONSTRUCT_LOCATION(@$), tinyc::UnaryOp::op_sub);
	} 
	| "!" {
		$$ = std::make_unique<tinyc::UnaryOp>(CONSTRUCT_LOCATION(@$), tinyc::UnaryOp::op_not);
	};

L3Expr
	: UnaryExpr {
		assert_same_ptr(tinyc::UnaryExpr, $1);
		$$ = std::make_unique<tinyc::L3Expr>(CONSTRUCT_LOCATION(@$), std::move($1));
	}
	| L3Expr L3Op UnaryExpr {
		assert_same_ptr(tinyc::L3Expr, $1);
		assert_same_ptr(tinyc::L3Op, $2);
		assert_same_ptr(tinyc::UnaryExpr, $3);
		$$ = std::make_unique<tinyc::L3Expr>(CONSTRUCT_LOCATION(@$), std::move($1), std::move($2), std::move($3));
	};

L3Op
	: "*"  {
		$$ = std::make_unique<tinyc::L3Op>(CONSTRUCT_LOCATION(@$), tinyc::Operator::op_mul);
	}
	| "/"  {
		$$ = std::make_unique<tinyc::L3Op>(CONSTRUCT_LOCATION(@$), tinyc::Operator::op_div);
	}
	| "%" {
		$$ = std::make_unique<tinyc::L3Op>(CONSTRUCT_LOCATION(@$), tinyc::Operator::op_mod);
	};

L4Expr
	: L3Expr {
		assert_same_ptr(tinyc::L3Expr, $1);
		$$ = std::make_unique<tinyc::L4Expr>(CONSTRUCT_LOCATION(@$), std::move($1));
	}
	| L4Expr L4Op L3Expr {
		assert_same_ptr(tinyc::L4Expr, $1);
		assert_same_ptr(tinyc::L4Op, $2);
		assert_same_ptr(tinyc::L3Expr, $3);
		$$ = std::make_unique<tinyc::L4Expr>(
			CONSTRUCT_LOCATION(@$),
			std::move($1),
			std::move($2),
			std::move($3)
		);
	};

L4Op
	: "+" {
		$$ = std::make_unique<tinyc::L4Op>(
			CONSTRUCT_LOCATION(@$),
			tinyc::Operator::op_add
		);
	}
	| "-" {
		$$ = std::make_unique<tinyc::L4Op>(CONSTRUCT_LOCATION(@$),
			tinyc::Operator::op_sub);
	};

L6Expr
	: L4Expr {
		assert_same_ptr(tinyc::L4Expr, $1);
		$$ = std::make_unique<tinyc::L6Expr>(
			CONSTRUCT_LOCATION(@$),
			std::move($1)
		);
	}
	| L6Expr L6Op L4Expr {
		assert_same_ptr(tinyc::L6Expr, $1);
		assert_same_ptr(tinyc::L6Op, $2);
		assert_same_ptr(tinyc::L4Expr, $3);
		$$ = std::make_unique<tinyc::L6Expr>(CONSTRUCT_LOCATION(@$), std::move($1), std::move($2), std::move($3));
	};

L6Op
	: "<" {
		$$ = std::make_unique<tinyc::L6Op>(CONSTRUCT_LOCATION(@$), tinyc::Operator::op_lt);
	}
	| ">" {
		$$ = std::make_unique<tinyc::L6Op>(CONSTRUCT_LOCATION(@$), tinyc::Operator::op_gt);
	}
	| "<=" {
		$$ = std::make_unique<tinyc::L6Op>(CONSTRUCT_LOCATION(@$), tinyc::Operator::op_le);
	}
	| ">=" {
		$$ = std::make_unique<tinyc::L6Op>(CONSTRUCT_LOCATION(@$), tinyc::Operator::op_ge);
	};

L7Expr      
	: L6Expr {
		assert_same_ptr(tinyc::L6Expr, $1);
		$$ = std::make_unique<tinyc::L7Expr>(CONSTRUCT_LOCATION(@$), std::move($1));
	}
	| L7Expr L7Op L6Expr {
		assert_same_ptr(tinyc::L7Expr, $1);
		assert_same_ptr(tinyc::L7Op, $2);
		assert_same_ptr(tinyc::L6Expr, $3);
		$$ = std::make_unique<tinyc::L7Expr>(CONSTRUCT_LOCATION(@$), std::move($1), std::move($2), std::move($3));
	};

L7Op		
	: "==" {
		$$ = std::make_unique<tinyc::L7Op>(CONSTRUCT_LOCATION(@$), tinyc::Operator::op_eq);
	}
	| "!=" {
		$$ = std::make_unique<tinyc::L7Op>(CONSTRUCT_LOCATION(@$), tinyc::Operator::op_ne);
	};

LAndExpr	
	: L7Expr {
		assert_same_ptr(tinyc::L7Expr, $1);
		$$ = std::make_unique<tinyc::LAndExpr>(CONSTRUCT_LOCATION(@$), std::move($1));
	}
	| LAndExpr LAndOp L7Expr{
		assert_same_ptr(tinyc::LAndExpr, $1);
		assert_same_ptr(tinyc::LAndOp, $2);
		assert_same_ptr(tinyc::L7Expr, $3);
		$$ = std::make_unique<tinyc::LAndExpr>(CONSTRUCT_LOCATION(@$), std::move($1), std::move($2), std::move($3));
	};

LAndOp		
	: "&&" {
		$$ = std::make_unique<tinyc::LAndOp>(CONSTRUCT_LOCATION(@$), tinyc::Operator::op_land);
	}

LOrExpr
	: LAndExpr {
		assert_same_ptr(tinyc::LAndExpr, $1);
		$$ = std::make_unique<tinyc::LOrExpr>(CONSTRUCT_LOCATION(@$), std::move($1));
	}
	| LOrExpr LOrOp LAndExpr {
		assert_same_ptr(tinyc::LOrExpr, $1);
		assert_same_ptr(tinyc::LOrOp, $2);
		assert_same_ptr(tinyc::LAndExpr, $3);
		$$ = std::make_unique<tinyc::LOrExpr>(CONSTRUCT_LOCATION(@$), std::move($1), std::move($2), std::move($3));
	};

LOrOp	
	: "||" {
		$$ = std::make_unique<tinyc::LOrOp>(CONSTRUCT_LOCATION(@$), tinyc::Operator::op_lor);
	};

Number
	: INT_LITERAL{
		$$ = std::make_unique<tinyc::Number>(CONSTRUCT_LOCATION(@$), $1);
	};

Ident
	: IDENT{
		$$ = std::make_unique<tinyc::Ident>(CONSTRUCT_LOCATION(@$), $1);
	};

%%

namespace yy
{

void parser::error(const location_type& loc, const std::string& m)
{
	loc.report(tinyc::Location::dk_error, m);
}

}	//namespace yy

