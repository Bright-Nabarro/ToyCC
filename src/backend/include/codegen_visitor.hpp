#pragma once
#include "codegen_context.hpp"

#include "ast.hpp"
#include "symbol_table.hpp"

namespace toycc
{

class CodeGenVisitor: public ASTVisitor, public CGContextInterface
{
public:
	CodeGenVisitor(std::shared_ptr<CodeGenContext> cg_context);
	/// @note 只支持从根节点翻译
	[[nodiscard]]
	auto visit(BaseAST* ast) -> std::expected<void, std::string> override;

private:
	using SymbolTable = std::unordered_map<std::string_view, llvm::Value*>;

	void handle(const CompUnit& node);
	void handle(const Module& node);
	void handle(const FuncDef& node);

	auto handle(const BuiltinType& node) -> llvm::Type*;
	auto handle(const ScalarType& node) -> llvm::Type*;

	auto handle(const ParamList& node)
		-> std::pair<std::vector<std::string_view>, std::vector<llvm::Type*>>;

	auto create_basic_block(const Block& node, llvm::Function* func,
				std::string_view block_name, std::span<std::string_view> param_names) -> llvm::BasicBlock*;
	// 不创建新块的情况
	void handle(const Block& node, LocalSymbolTable& upper_table);

	void handle(const BlockItemList& node, LocalSymbolTable& table);
	void handle(const BlockItem& node, LocalSymbolTable& table);
	
	auto handle(const Param& node) -> std::pair<std::string_view, llvm::Type*>;

	auto handle(const Number& num) -> llvm::Value*;
	auto handle(const Ident& node) -> std::string_view;

	void handle(const Decl& node, LocalSymbolTable& table);
	void handle(const ConstDecl& node, LocalSymbolTable& table);

	void handle(const Stmt& node, LocalSymbolTable& table);
	void handle(const SelectStmt& node, LocalSymbolTable& table);
	auto handle(const Expr& expr, LocalSymbolTable& table) -> llvm::Value*;
	auto handle(const PrimaryExpr& node, LocalSymbolTable& table)
		-> llvm::Value*;
	auto handle(const UnaryExpr& node, LocalSymbolTable& table) -> llvm::Value*;
	auto handle(const PassingParams& node, LocalSymbolTable& table)
		-> std::vector<llvm::Value*>;
	/// @return false代表用户出错返回
	auto handle(const ExprList& node, LocalSymbolTable& table,
				std::vector<llvm::Value*>& list) -> bool;

	void handle(const ConstDef& node, llvm::Type* type, LocalSymbolTable& table);
	void handle(const ConstDefList& node, llvm::Type* type, LocalSymbolTable& table);
	auto handle(const ConstInitVal& node, LocalSymbolTable& table)
		-> llvm::Value*;
	auto handle(const ConstExpr& node, LocalSymbolTable& table) -> llvm::Value*;
	/**
	 * @return 如果无法查找到返回nullptr
	 */
	auto handle(const LVal& node, LocalSymbolTable& table)
		-> std::shared_ptr<SymbolEntry>;
	
	void handle(const VarDecl& node, LocalSymbolTable& table);
	void handle(const VarDef& node, llvm::Type* type, LocalSymbolTable& table);
	void handle(const VarDefList& node, llvm::Type* type, LocalSymbolTable& table);
	auto handle(const InitVal& node, LocalSymbolTable& table) -> llvm::Value*;


	/// @note 在上层会传入所有的BinaryExpr, 无需在实现文件中显式实例化声明
	template <typename TBinaryExpr>
		requires std::derived_from<TBinaryExpr, BinaryExprBase>
	auto handle(const TBinaryExpr& node, LocalSymbolTable& table)
		-> llvm::Value*;

	/// @brief 一元运算符处理
	auto unary_operate(const UnaryOp& op, llvm::Value* operand) -> llvm::Value*;
	/// @brief 二元运算符通用处理函数
	auto binary_operate(llvm::Value* left, const Operator& op,
						llvm::Value* right) -> llvm::Value*;

	auto report_conversion_result(const ConversionResult& result,
								  const BaseAST& node) -> llvm::Type*;
	void report_in_ast(const BaseAST& node, Location::DiagKind kind,
					   std::string_view msg);

private:
	std::shared_ptr<GlobalSymbolTable> m_global_table;
};

}	//namespace toycc

