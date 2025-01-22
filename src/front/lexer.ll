%{
#include <cerrno>
#include <cstring> 	//strerror
#include <cstdlib>
#include "driver.hpp"
#include "bison_parser.hpp"

#define LOC_UPDATE_RET_ACTION(loc, action)                                     \
	do                                                                         \
	{                                                                          \
		loc.update(yyleng);                                                    \
		return (action);                                                       \
	} while (0)

#define LOC_UPDATE_NORMAL(loc)                                                 \
	do                                                                         \
	{                                                                          \
		loc.update(yyleng);                                                    \
		loc.step();                                                            \
	} while(0)

%}

%option noyywrap nounput noinput batch debug

blank	 		[ \t\r\n]+
LineComment		\/\/[^\n]*\n
LegacyComment 	\/\*.*\*\/
Ident			[a-zA-Z_][0-9a-zA-Z_]*
Number			[0-9]+
SignedInt		(signed\s+int)|(int)|(signed)
UnsignedInt		(unsigned\s+int)|(unsigned)
%%

%{
	//在%% %%中间插入的代码每次yylex执行时调用
	//如果之前已经步进过，则此时不会改变
	auto& loc = driver.get_location();
	loc.step();
%}


{blank}			LOC_UPDATE_NORMAL(loc);
{LineComment}	LOC_UPDATE_NORMAL(loc);
{LegacyComment} LOC_UPDATE_NORMAL(loc);

{SignedInt}		LOC_UPDATE_RET_ACTION(loc, yy::parser::make_KW_SINT(loc));
{UnsignedInt}	LOC_UPDATE_RET_ACTION(loc, yy::parser::make_KW_UINT(loc));
"void"			LOC_UPDATE_RET_ACTION(loc, yy::parser::make_KW_VOID(loc));
"return"		LOC_UPDATE_RET_ACTION(loc, yy::parser::make_KW_RETURN(loc)); 
"const"			LOC_UPDATE_RET_ACTION(loc, yy::parser::make_KW_CONST(loc));

{Ident}			LOC_UPDATE_RET_ACTION(loc, yy::parser::make_IDENT(std::string{yytext}, loc));
{Number}		LOC_UPDATE_RET_ACTION(loc, yy::parser::make_INT_LITERAL(std::atoi(yytext), loc));
"("				LOC_UPDATE_RET_ACTION(loc, yy::parser::make_DELIM_LPAREN(loc));
")"				LOC_UPDATE_RET_ACTION(loc, yy::parser::make_DELIM_RPAREN(loc));
"{"				LOC_UPDATE_RET_ACTION(loc, yy::parser::make_DELIM_LBRACE(loc));
"}"				LOC_UPDATE_RET_ACTION(loc, yy::parser::make_DELIM_RBRACE(loc));
","				LOC_UPDATE_RET_ACTION(loc, yy::parser::make_DELIM_COMMA(loc));
";"				LOC_UPDATE_RET_ACTION(loc, yy::parser::make_DELIM_SEMICOLON(loc));
"+"				LOC_UPDATE_RET_ACTION(loc, yy::parser::make_OP_ADD(loc));
"-" 			LOC_UPDATE_RET_ACTION(loc, yy::parser::make_OP_SUB(loc));
"!" 			LOC_UPDATE_RET_ACTION(loc, yy::parser::make_OP_NOT(loc));
"*"				LOC_UPDATE_RET_ACTION(loc, yy::parser::make_OP_MUL(loc));
"/"				LOC_UPDATE_RET_ACTION(loc, yy::parser::make_OP_DIV(loc));
"%"				LOC_UPDATE_RET_ACTION(loc, yy::parser::make_OP_MOD(loc));
"<"				LOC_UPDATE_RET_ACTION(loc, yy::parser::make_OP_LT(loc));
"<="			LOC_UPDATE_RET_ACTION(loc, yy::parser::make_OP_LE(loc));
">"				LOC_UPDATE_RET_ACTION(loc, yy::parser::make_OP_GT(loc));
">="			LOC_UPDATE_RET_ACTION(loc, yy::parser::make_OP_GE(loc));
"=="			LOC_UPDATE_RET_ACTION(loc, yy::parser::make_OP_EQ(loc));
"!="			LOC_UPDATE_RET_ACTION(loc, yy::parser::make_OP_NE(loc));
"&&"			LOC_UPDATE_RET_ACTION(loc, yy::parser::make_OP_LAND(loc));
"||"			LOC_UPDATE_RET_ACTION(loc, yy::parser::make_OP_LOR(loc));
"="				LOC_UPDATE_RET_ACTION(loc, yy::parser::make_OP_ASSIGN(loc));
.				{
					driver.get_parser().error(loc, "expect token");
					return yy::parser::make_YYerror(loc);
				}
				
<<EOF>>			return yy::parser::make_YYEOF(loc);

%%


namespace tinyc
{

void Driver::set_flex(const char* buffer, int buffer_size)
{
	yy_flex_debug = this->get_trace();
	yy_scan_bytes(buffer, buffer_size);
}

}	//namespace tinyc

