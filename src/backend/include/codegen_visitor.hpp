#pragma once

#include "ast.hpp"
#include "type_mgr.hpp"
#include "conversion.hpp"
#include "symbol_table.hpp"
#include <memory>
#include <expected>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Support/SourceMgr.h>
#include <spdlog/async.h>

namespace toycc
{

class CodeGenVisitor: public ASTVisitor
{
public:
	CodeGenVisitor(llvm::LLVMContext& context,
				   std::shared_ptr<ConversionHelper> cvt_helper,
				   llvm::SourceMgr& src_mgr, llvm::TargetMachine* tm,
				   std::shared_ptr<spdlog::async_logger> logger);
	/// @note 只支持从根节点翻译
	[[nodiscard]]
	auto visit(BaseAST* ast) -> std::expected<void, std::string> override;

	/// @brief 在语法解析后获取结果
	[[nodiscard]]
	auto get_module() -> std::unique_ptr<llvm::Module>
	{ return std::move(m_module); }

private:
	using SymbolTable = std::unordered_map<std::string_view, llvm::Value*>;

	void handle(const CompUnit& node);
	void handle(const FuncDef& node);

	auto handle(const BuiltinType& node) -> llvm::Type*;
	auto handle(const ScalarType& node) -> llvm::Type*;

	auto handle(const ParamList& node) -> std::vector<llvm::Type*>;

	auto handle(const Block& node, llvm::Function* func,
				std::string_view block_name) -> llvm::BasicBlock*;

	void handle(const BlockItemList& node, LocalSymbolTable& table);
	void handle(const BlockItem& node, LocalSymbolTable& table);
	
	auto handle(const Param& node) -> llvm::Type*;

	auto handle(const Number& num) -> llvm::Value*;
	auto handle(const Ident& node) -> std::string_view;

	void handle(const Decl& node, LocalSymbolTable& table);
	auto handle(const ConstDecl& node, LocalSymbolTable& table)
		-> std::vector<llvm::Value*>; 

	void handle(const Stmt& node, LocalSymbolTable& table);
	auto handle(const Expr& expr, LocalSymbolTable& table) -> llvm::Value*;
	auto handle(const PrimaryExpr& node, LocalSymbolTable& table)
		-> llvm::Value*;
	auto handle(const UnaryExpr& node, LocalSymbolTable& table) -> llvm::Value*;

	auto handle(const ConstDef& node, llvm::Type* type, LocalSymbolTable& table)
		-> llvm::Value*;
	auto handle(const ConstDefList& node, llvm::Type* type,
				LocalSymbolTable& table) -> std::vector<llvm::Value*>;
	auto handle(const ConstInitVal& node, LocalSymbolTable& table)
		-> llvm::Value*;
	auto handle(const ConstExpr& node, LocalSymbolTable& table) -> llvm::Value*;
	/// @return 如果无法查找到返回nullptr
	auto handle(const LVal& node, LocalSymbolTable& table) -> llvm::Value*;
	
	void handle(const VarDecl& node, LocalSymbolTable& table);
	void handle(const VarDef& node, LocalSymbolTable& table);
	void handle(const VarDefList& node, LocalSymbolTable& table);
	void handle(const InitVal& node, LocalSymbolTable& table);

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

private:
	std::unique_ptr<llvm::Module> m_module;
	llvm::IRBuilder<> m_builder;
	std::shared_ptr<TypeMgr> m_type_mgr;
	std::shared_ptr<ConversionHelper> m_cvt_helper;

	llvm::SourceMgr& m_src_mgr;
	llvm::TargetMachine* m_target_machine;
	std::shared_ptr<spdlog::async_logger> m_logger;
};

}	//namespace toycc

