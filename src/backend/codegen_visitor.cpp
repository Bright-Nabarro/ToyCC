#include "codegen_visitor.hpp"

#include <format>
#include <print>

#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/CodeGen/CommandFlags.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Pass.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Transforms/Scalar.h>

namespace toycc
{

CodeGenVisitor::CodeGenVisitor(llvm::LLVMContext& context,
							   std::shared_ptr<ConversionHelper> cvt_helper,
							   llvm::SourceMgr& src_mgr,
							   llvm::TargetMachine* tm,
							   std::shared_ptr<spdlog::async_logger> logger)
	: m_module{std::make_unique<llvm::Module>("toycc.expr", context)},
	  m_builder{m_module->getContext()},
	  m_type_mgr{std::make_shared<TypeMgr>(m_module->getContext(), tm)},
	  m_cvt_helper { cvt_helper },
	  m_src_mgr{src_mgr}, m_target_machine{tm}, m_logger{logger},
	  m_global_table { std::make_shared<GlobalSymbolTable>() }
{
}

auto CodeGenVisitor::visit(BaseAST* ast) -> std::expected<void, std::string>
{
	if (ast == nullptr)
	{
		return std::unexpected {std::format("{}", "visit paramater is null")};
	}

	auto comp_unit_ptr = llvm::dyn_cast<CompUnit>(ast);
	if (comp_unit_ptr == nullptr)
	{
		return std::unexpected {std::format("{}", "output visitor paramater should be a CompUnit") };
	}
	
	handle(*comp_unit_ptr);

	return {};
}

#define D_BEGIN                                                                \
	do                                                                         \
	{                                                                          \
		m_logger->debug("{} Begin", node.get_kind_str());                      \
	} while (0);

#define D_END                                                                  \
	do                                                                         \
	{                                                                          \
		m_logger->debug("{} End", node.get_kind_str());                        \
	} while (0);

void CodeGenVisitor::handle(const CompUnit& node)
{
	D_BEGIN;
	handle(node.get_module());
	D_END;
}

void CodeGenVisitor::handle(const Module& node)
{
	D_BEGIN;
	switch(node.get_type())
	{
	case Module::extern_func:
		if (node.has_next_module())
			handle(node.get_module());
		handle(node.get_func_def());
		break;
	case Module::extern_global_variable:
		if (node.has_next_module())
			handle(node.get_module());
		handle(node.get_module());
		break;
	default:
		assert(false && "Unsupport");
	}
	D_END;
}

void CodeGenVisitor::handle(const FuncDef& node)
{
	D_BEGIN;
	auto return_type = handle(node.get_type());
	auto func_name = handle(node.get_ident());
	auto param_types = handle(node.get_paramlist());

	auto func_type = llvm::FunctionType::get(return_type, param_types, false);

	auto func =
		llvm::Function::Create(func_type, llvm::GlobalValue::ExternalLinkage,
							   func_name, m_module.get());

	create_basic_block(node.get_block(), func, "entry");

	D_END;
}

auto CodeGenVisitor::handle(const BuiltinType& node) -> llvm::Type*
{
	D_BEGIN;

	llvm::Type* ret;
	switch(node.get_type())
	{
	case toycc::BuiltinTypeEnum::ty_signed_int:
		ret = m_type_mgr->get_signed_int();
		break;
	case toycc::BuiltinTypeEnum::ty_unsigned_int:
		ret = m_type_mgr->get_unsigned_int();
		break;
	case toycc::BuiltinTypeEnum::ty_void:
		ret = m_type_mgr->get_void();
		break;
	default:
		assert(false &&"toycc::Type to llvm::Type*");
	}

	D_END;

	return ret;
}

auto CodeGenVisitor::handle(const ScalarType& node) -> llvm::Type*
{
	D_BEGIN;
	llvm::Type* ret;
	switch(node.get_type())
	{
	case toycc::BuiltinTypeEnum::ty_signed_int:
		ret = m_type_mgr->get_signed_int();
		break;
	case toycc::BuiltinTypeEnum::ty_unsigned_int:
		ret = m_type_mgr->get_unsigned_int();
		break;
	default:
		assert(false && "Unkown TypeEnum int toycc::Type when handling "
			  "toycc::Type to llvm::Type*");
	}
	D_END;
	return ret;
}


auto CodeGenVisitor::handle(const Ident& node) -> std::string_view
{
	D_BEGIN;
	std::string_view name = node.get_value();
	D_END;
	return name;
}

auto CodeGenVisitor::handle(const ParamList& node) -> std::vector<llvm::Type*>
{
	D_BEGIN;
	std::vector<llvm::Type*> type_list;
	type_list.reserve(node.get_params().size());

	for (const auto& param : node)
	{
		assert(param != nullptr);
		type_list.push_back(handle(*param));
	}
	D_END;

	return type_list;
}

auto CodeGenVisitor::create_basic_block(const Block& node, llvm::Function* func,
							std::string_view block_name) -> llvm::BasicBlock*
{
	D_BEGIN;

	auto basic_block =
		llvm::BasicBlock::Create(m_module->getContext(), block_name.data(), func);

	// 局部符号表
	LocalSymbolTable table(func, m_global_table);

	m_builder.SetInsertPoint(basic_block);
	handle(node.get_block_item_list(), table);

	D_END;
	return basic_block;
}

void CodeGenVisitor::handle(const Block& node, LocalSymbolTable& upper_table)
{
	LocalSymbolTable table { upper_table };
	handle(node.get_block_item_list(), table);
}

void CodeGenVisitor::handle(const BlockItemList& node, LocalSymbolTable& table)
{
	D_BEGIN;
	for (const auto& block_item : node)
	{
		assert(block_item != nullptr);
		handle(*block_item, table);
	}
	D_END;
}

void CodeGenVisitor::handle(const BlockItem& node, LocalSymbolTable& table)
{
	D_BEGIN;
	if (node.has_decl())
		handle(node.get_decl(), table);
	else if (node.has_stmt())
		handle(node.get_stmt(), table);
	D_END;
}

void CodeGenVisitor::handle(const Stmt& node, LocalSymbolTable& table)
{
	D_BEGIN;
	switch (node.get_type())
	{
	case Stmt::assign:
	{
		auto left_entry = handle(node.get_lval(), table);
		llvm::Value* right_value = handle(node.get_expr(), table);

		if (left_entry->type != SymbolEntry::alloca_value) [[unlikely]]
		{
			report_in_ast(node, Location::DiagKind::dk_error,
						  "An eval value cannot be assigned");
			break;
		}

		if (left_entry == nullptr) [[unlikely]]
		{
			m_logger->info("User Error occured in Stmt::assign left value");
			break;
		}
		if (right_value == nullptr) [[unlikely]]
		{
			m_logger->info("User Error occured in Stmt::assign right value");
			break;
		}

		m_builder.CreateStore(right_value, left_entry->alloca);
		
		break;
	}
	case Stmt::expression:
	{
		if (!node.has_expr())
		{
			m_logger->debug("Empty Stmt expression");
			break;
		}

		auto value = handle(node.get_expr(), table);
		if (value == nullptr)
		{
			m_logger->info("User Error occured in Stmt::expression");
		}
		break;
	}
	case Stmt::block:
	{
		handle(node.get_block(), table);
		break;
	}
	case Stmt::func_return:
	{
		if (!node.has_expr())
		{
			m_logger->debug("Empty return statement");
			m_builder.CreateRetVoid();
			break;
		}
		auto value = handle(node.get_expr(), table);
		if (value == nullptr)
		{
			m_logger->info("User Error occured in Stmt::func_return");
		}
		m_builder.CreateRet(value);
		break;
	}
	case Stmt::if_stmt:
	{
		llvm::BasicBlock* if_then = llvm::BasicBlock::Create(
			m_module->getContext(), "", table.get_func());
		llvm::BasicBlock* if_end = llvm::BasicBlock::Create(
			m_module->getContext(), "", table.get_func());

		auto cmp = handle(node.get_expr(), table);
		if (!m_cvt_helper->convert_to_bool(cmp->getType()))
		{
			report_in_ast(node, Location::DiagKind::dk_error, "Cannot convert to bool");
			break;
		}

		const auto& stmts = node.get_stmts();

		if (stmts.size() == 1) // if 语句
		{
			m_builder.CreateCondBr(cmp, if_then, if_end);
			m_builder.SetInsertPoint(if_then);
			handle(*stmts.front(), table);
			m_builder.CreateBr(if_end);
		}
		else if(stmts.size() == 2) // if else 语句
		{
			llvm::BasicBlock* if_else = llvm::BasicBlock::Create(
				m_module->getContext(), "", table.get_func());
			m_builder.CreateCondBr(cmp, if_then, if_else);
			m_builder.SetInsertPoint(if_then);
			handle(*stmts[0], table);
			m_builder.SetInsertPoint(if_else);
			handle(*stmts[1], table);
			m_builder.CreateBr(if_end);
		}
		else
		{
			assert(false);
		}

		m_builder.SetInsertPoint(if_end);
		break;
	}
	case Stmt::while_stmt:
	{
		llvm::BasicBlock* cond = llvm::BasicBlock::Create(
			m_module->getContext(), "", table.get_func());
		llvm::BasicBlock* body = llvm::BasicBlock::Create(
			m_module->getContext(), "", table.get_func());
		llvm::BasicBlock* end = llvm::BasicBlock::Create(
			m_module->getContext(), "", table.get_func());

		m_builder.CreateBr(cond);
		// 条件判断 begin
		m_builder.SetInsertPoint(cond);
		auto value = handle(node.get_expr(), table);
		m_builder.CreateCondBr(value, body, end);
		// 条件判断 end
		// 循环体
		m_builder.SetInsertPoint(body);
		const auto& stmt_body = node.get_stmts();
		assert(stmt_body.size() == 1);
		handle(*stmt_body.front(), table);
		m_builder.CreateBr(cond);
		// 循环体 end
		m_builder.SetInsertPoint(end);
		break;
	}
	default:
		m_logger->error("Unkown StmtType");
		std::abort();
	}
	
	D_END;
}

auto CodeGenVisitor::handle(const Expr& node, LocalSymbolTable& table)
	-> llvm::Value*
{
	D_BEGIN;
	auto ret = handle(node.get_low_expr(), table);
	D_END;

	return ret;
}

auto CodeGenVisitor::handle(const PrimaryExpr& node, LocalSymbolTable& table)
	-> llvm::Value*
{
	D_BEGIN;

	llvm::Value* result = nullptr;
	if (node.has_expr())
	{
		result = handle(node.get_expr(), table);	
	}
	else if (node.has_ident())
	{
		auto entry = handle(node.get_lval(), table);

		if (entry == nullptr)
		{
			m_logger->info("User Error Occured in LVal");
		}
		else
		{
			result = entry->type == SymbolEntry::eval_value ?
				entry->value :
				m_builder.CreateLoad(entry->alloca->getAllocatedType(), entry->alloca);
		}
	}
	else if (node.has_number())
	{
		result = handle(node.get_number());
	}
	
	if (result == nullptr)
	{
		m_logger->info("User Error Occured in PrimaryExpr");
	}

	D_END;
	return result;
}

auto CodeGenVisitor::handle(const UnaryExpr& node, LocalSymbolTable& table)
	-> llvm::Value*
{
	static auto handle_func = [this](const UnaryExpr& node) -> llvm::Function*
	{
		auto func_name = handle(node.get_ident());
		auto entry = m_global_table->find(func_name);
		if (!entry)
		{
			report_in_ast(node, Location::dk_error,
						  std::format("Cannot find function {}", func_name));
			return nullptr;
		}
		if (entry->type != SymbolEntry::func_value)
		{
			report_in_ast(
				node, Location::dk_error,
				std::format("Value {} is not a function type", func_name));
		}
		llvm::Function* func = llvm::cast<llvm::Function>(entry->value);
		return func;
	};

	D_BEGIN;
	
	llvm::Value* result = nullptr;
	switch(node.get_unary_type())
	{
	case UnaryExpr::primary_expr:
		result = handle(node.get_primary_expr(), table);
		break;
	case UnaryExpr::unary_op:
		result = handle(node.get_unary_expr(), table);
		result = unary_operate(node.get_unary_op(), result);
		break;
	case UnaryExpr::call_with_params: {
		auto func = handle_func(node);
		if (!func)
			return nullptr;
		auto passing_list = handle(node.get_passing_params(), table);

		m_builder.CreateCall(func, passing_list);
		break;
	}
	case UnaryExpr::call: {
		auto func = handle_func(node);
		if (!func)
			return nullptr;

		m_builder.CreateCall(func);
		break;
	}
	default:
		assert(false && "UnaryExpr has an unkown type");
	}

	D_END;

	return result;
}

auto CodeGenVisitor::handle(const PassingParams& node, LocalSymbolTable& table)
		-> std::vector<llvm::Value*>
{
	D_BEGIN;
	std::vector<llvm::Value*> result;
	result.reserve(node.size());
	result.push_back(handle(node.get_expr(), table));
	handle(node.get_expr_list(), table, result);

	D_END;

	return result;
}

auto CodeGenVisitor::handle(const ExprList& node, LocalSymbolTable& table,
							std::vector<llvm::Value*>& list) -> bool
{
	D_BEGIN;
	for (const auto& expr: node)
	{
		auto ret = handle(*expr, table);
		if (!ret)
			return false;
		list.push_back(ret);
	}

	D_END;

	return true;
}

auto CodeGenVisitor::handle(const Number& node) -> llvm::Value*
{
	D_BEGIN;

	llvm::Value* result = llvm::ConstantInt::get(m_type_mgr->get_signed_int(),
											 node.get_int_literal());
	D_END;
	return result;
}

void CodeGenVisitor::handle(const Decl& node, LocalSymbolTable& table)
{
	D_BEGIN;

	if (node.has_const_decl())
		handle(node.get_const_decl(), table);
	else if (node.has_var_decl())
		handle(node.get_var_decl(), table);
	else
		assert(false && "Unkown type in Decl");

	D_END;
}

void CodeGenVisitor::handle(const ConstDecl& node, LocalSymbolTable& table)
{
	D_BEGIN;

	llvm::Type* type = handle(node.get_scalar_type());
	handle(node.get_first_const_def(), type, table);
	handle(node.get_const_def_list(), type, table);

	D_END;
}

void CodeGenVisitor::handle(const ConstDef& node, llvm::Type* type,
							LocalSymbolTable& table)
{
	D_BEGIN;

	auto name_str = handle(node.get_ident());

	llvm::Value* right_value = handle(node.get_const_init_val(), table);
	
	auto ret = m_cvt_helper->value_conversion(
		type, right_value->getType()
	);
	
	auto left_type = report_conversion_result(ret, node);
	if (left_type == nullptr)
		return;
	
	llvm::Value* left_value = right_value;
	left_value->mutateType(left_type);

	auto entry = std::make_shared<SymbolEntry>(SymbolEntry::eval_value, left_value);
	if (!table.insert(name_str, entry))
	{
		report_in_ast(node, Location::dk_error,
				std::format("Variable {} has been defined", name_str));
		return;
	}
	
	D_END;
}

void CodeGenVisitor::handle(const ConstDefList& node, llvm::Type* type,
							LocalSymbolTable& table)
{
	D_BEGIN;
	for (const auto& const_def_ptr : node )
	{
		handle(*const_def_ptr, type, table);
	}
	D_END;
}

auto CodeGenVisitor::handle(const ConstInitVal& node, LocalSymbolTable& table)
	-> llvm::Value*
{
	D_BEGIN;
	auto ret = handle(node.get_const_expr(), table);
	D_END;
	return ret;
}

auto CodeGenVisitor::handle(const ConstExpr& node, LocalSymbolTable& table)
	-> llvm::Value*
{
	D_BEGIN;
	auto result = handle(node.get_expr(), table);
	D_END;

	return result;
}

auto CodeGenVisitor::handle(const LVal& node, LocalSymbolTable& table)
	-> std::shared_ptr<SymbolEntry>
{
	D_BEGIN;
	auto name = handle(node.get_id());
	auto entry = table.lookup(name);
	if (entry == nullptr)
	{
		report_in_ast(node, Location::dk_error,
					std::format("Variable {} not defined", name));
		return nullptr;
	}

	D_END;
	
	return entry;
}

auto CodeGenVisitor::unary_operate(const UnaryOp& op, llvm::Value* operand)
	-> llvm::Value*
{
	m_logger->debug("UnaryOp[{}] Begin", op.get_type_str());
	llvm::Value* result = nullptr;
	llvm::Type* type = operand->getType();

	if (!type->isIntegerTy() && !type->isFloatingPointTy())
	{
		m_logger->error("Expected type in UnaryOp");
		return nullptr;
	}

	switch (op.get_type())
	{
	case UnaryOp::op_add:
		result = operand;
		break;
	case UnaryOp::op_sub:
		if (type->isIntegerTy())
			result = m_builder.CreateNeg(operand);
		else 
			result = m_builder.CreateFNeg(operand);
		break;
	/// c语言not操作将操作数转换为int类型
	case UnaryOp::op_not:
	{
		llvm::Value* zero = llvm::ConstantInt::get(type, 0);
		llvm::Value* is_nonzero = nullptr;
		if (type->isIntegerTy())
		{
			is_nonzero = m_builder.CreateICmpNE(operand, zero);
		}
		else
		{
			is_nonzero = m_builder.CreateFCmpUNE(operand, zero);
		}
		llvm::Value* int_value =
			m_builder.CreateZExt(is_nonzero, m_type_mgr->get_signed_int());

		result = m_builder.CreateNot(int_value);
		break;
	}
	default:
		m_logger->error("Unkown operation: {}, category: {}",
						static_cast<int>(op.get_type()), op.get_type_str());
		result = nullptr;
	}

	m_logger->debug("UnaryOp[{}] Begin", op.get_type_str());
	return result;
}

auto CodeGenVisitor::handle(const Param& node) -> llvm::Type*
{
	D_BEGIN;
	auto type = handle(node.get_type());
	handle(node.get_ident());
	D_END;

	return type;
}

template <typename TBinaryExpr>
	requires std::derived_from<TBinaryExpr, BinaryExprBase>
auto CodeGenVisitor::handle(const TBinaryExpr& node, LocalSymbolTable& table)
	-> llvm::Value*
{
	D_BEGIN;

	llvm::Value* result = nullptr;

	if (node.has_higher_expr())
	{
		result = handle(node.get_higher_expr(), table);
	}
	else if (node.has_combined_expr())
	{
		auto [self_expr_ref, op, higher_expr_ref ] = node.get_combined_expr();
		auto left = handle(self_expr_ref.get(), table);
		auto right = handle(higher_expr_ref.get(), table);
		
		result = binary_operate(left, op, right);
	}
	else
	{
		assert(false && "Variant has an unkown type");
	}
	D_END;

	return result;
}

auto CodeGenVisitor::report_conversion_result(const ConversionResult& result,
											  const BaseAST& node)
	-> llvm::Type*
{
	switch(result.status)
	{
	case ConversionStatus::success:
		return result.result_type;
	case ConversionStatus::warning:
		report_in_ast(node, Location::dk_warning, result.ec.message());
		return result.result_type;
	case ConversionStatus::failure:
		report_in_ast(node, Location::dk_warning, result.ec.message());
		return nullptr;
	default:
		assert(false);
	}
}

auto CodeGenVisitor::binary_operate(llvm::Value* left, const Operator& op,
									llvm::Value* right) -> llvm::Value*
{
	m_logger->debug("{} [{}] Begin:", op.get_kind_str(), op.get_type_str());

	llvm::Value* result = nullptr;
	assert(left->getType() == right->getType());
	
	switch(op.get_type())
	{
	case Operator::op_add:
		result = m_builder.CreateAdd(left, right);
		break;
	case Operator::op_sub:
		result = m_builder.CreateSub(left, right);
		break;
	case Operator::op_mul:
		result = m_builder.CreateMul(left, right);
		break;
	case Operator::op_div:
		result = m_builder.CreateSDiv(left, right);
		break;
	case Operator::op_mod:
		result = m_builder.CreateSRem(left, right);
		break;
	case Operator::op_lt:
		result = m_builder.CreateICmpSLT(left, right);
		break;
	case Operator::op_le:
		result = m_builder.CreateICmpSLE(left, right);
		break;
	case Operator::op_gt:
		result = m_builder.CreateICmpSGT(left, right);
		break;
	case Operator::op_ge:
		result = m_builder.CreateICmpSGE(left, right);
		break;
	case Operator::op_eq:
		result = m_builder.CreateICmpEQ(left, right);
		break;
	case Operator::op_ne:
		result = m_builder.CreateICmpNE(left, right);
		break;
	case Operator::op_land:
		left = m_builder.CreateTrunc(left, llvm::Type::getInt1Ty(m_module->getContext()));
		right = m_builder.CreateTrunc(right, llvm::Type::getInt1Ty(m_module->getContext()));
		result = m_builder.CreateLogicalAnd(left, right);
		break;
	case Operator::op_lor:
		left = m_builder.CreateTrunc(left, llvm::Type::getInt1Ty(m_module->getContext()));
		right = m_builder.CreateTrunc(right, llvm::Type::getInt1Ty(m_module->getContext()));
		result = m_builder.CreateLogicalOr(left, right);
		break;
	default:
		//在二元运算符中
		assert(false && "Unprocessed binary operate");
	}

	assert(result != nullptr);
	m_builder.CreateZExt(result, m_type_mgr->get_signed_int());

	m_logger->debug("{} [{}] End", op.get_kind_str(), op.get_type_str());

	return result;
}

void CodeGenVisitor::handle(const VarDecl& node, LocalSymbolTable& table)
{
	D_BEGIN;

	auto* type = handle(node.get_scalar_type());
	handle(node.get_var_def(), type, table);
	handle(node.get_var_def_list(), type, table);

	D_END;
}

void CodeGenVisitor::handle(const VarDef& node, llvm::Type* type,
							LocalSymbolTable& table)
{
	D_BEGIN;

	auto name_str = handle(node.get_ident());

	// 创建一个新的局部变量，分配内存
	llvm::AllocaInst* alloca_inst =
		new llvm::AllocaInst(type, 0, name_str, m_builder.GetInsertBlock());

	if (node.is_initialized())
	{

		ConversionResult cvt_result;
		auto right_value = handle(node.get_init_val(), table);
		// 判断隐式类型转换是否合法
		cvt_result =
			m_cvt_helper->value_conversion(type, right_value->getType());
		auto left_type = report_conversion_result(cvt_result, node);
		if (left_type == nullptr)
			return;

		m_builder.CreateStore(right_value, alloca_inst);
	}
	// 在符号表中添加对应条目
	auto entry = std::make_shared<SymbolEntry>(alloca_inst);
	table.insert(name_str, entry);

	D_END;
}

void CodeGenVisitor::handle(const VarDefList& node, llvm::Type* type,
							LocalSymbolTable& table)
{
	D_BEGIN;
	for (const auto& ptr : node)
	{
		handle(*ptr, type, table);
	}

	D_END;
}

auto CodeGenVisitor::handle(const InitVal& node, LocalSymbolTable& table)
	-> llvm::Value*
{
	D_BEGIN;
	auto result = handle(node.get_expr(), table);
	D_END;

	return result;
}

void CodeGenVisitor::report_in_ast(const BaseAST& node, Location::DiagKind kind,
								   std::string_view msg)
{
	node.report(kind, msg, &m_src_mgr);
}

}	//namespace toycc


