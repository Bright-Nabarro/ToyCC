#pragma once
#include "codegen_context.hpp"
#include "ast.hpp"
#include "symbol_table.hpp"

namespace toycc
{

/**
 * @brief 
 */
class FunctionGenVisitor : public toycc::ASTVisitor, public CGContextInterface
{
public:
	FunctionGenVisitor(std::shared_ptr<CodeGenContext> cg_context);
private:	
	void handle(const FuncDef& node);

};


} 	//namespace toycc
