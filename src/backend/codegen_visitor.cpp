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

CodeGenVisitor::CodeGenVisitor(std::shared_ptr<CodeGenContext> cg_context):
	CGContextInterface { cg_context },
	m_success { true }
{}

auto CodeGenVisitor::visit(BaseAST* ast) -> bool
{
	if (ast == nullptr)
	{
		get_logger().error(std::format("{}", "visit paramater is null"));
		abort();
	}

	auto comp_unit_ptr = llvm::dyn_cast<CompUnit>(ast);
	if (comp_unit_ptr == nullptr)
	{
		get_logger().error(std::format("{}", "output visitor paramater should be a CompUnit"));
		abort();
	}
	
	handle(*comp_unit_ptr);

	return m_success;
}


void CodeGenVisitor::handle(const CompUnit& node)
{
	
	handle(node.get_module());
	
}

void CodeGenVisitor::handle(const Module& node)
{
	
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
	
}

void CodeGenVisitor::handle(const FuncDef& node)
{
	auto return_type = handle(node.get_type());
	auto func_name = handle(node.get_ident());
	auto [ param_names, param_types ] = handle(node.get_paramlist());
																	/* 不是可变类型 */
	auto func_type = llvm::FunctionType::get(return_type, param_types, false);

	auto func =
		llvm::Function::Create(func_type, llvm::GlobalValue::ExternalLinkage,
							   func_name, get_module());

	create_basic_block(node.get_block(), func, "entry", param_names);

}

auto CodeGenVisitor::handle(const BuiltinType& node) -> llvm::Type*
{
	

	llvm::Type* ret;
	switch(node.get_type())
	{
	case toycc::BuiltinTypeEnum::ty_signed_int:
		ret = get_type_mgr().get_signed_int();
		break;
	case toycc::BuiltinTypeEnum::ty_unsigned_int:
		ret = get_type_mgr().get_unsigned_int();
		break;
	case toycc::BuiltinTypeEnum::ty_void:
		ret = get_type_mgr().get_void();
		break;
	default:
		assert(false &&"toycc::Type to llvm::Type*");
	}

	

	return ret;
}

auto CodeGenVisitor::handle(const ScalarType& node) -> llvm::Type*
{
	
	llvm::Type* ret;
	switch(node.get_type())
	{
	case toycc::BuiltinTypeEnum::ty_signed_int:
		ret = get_type_mgr().get_signed_int();
		break;
	case toycc::BuiltinTypeEnum::ty_unsigned_int:
		ret = get_type_mgr().get_unsigned_int();
		break;
	default:
		assert(false && "Unkown TypeEnum int toycc::Type when handling "
			  "toycc::Type to llvm::Type*");
	}
	
	return ret;
}


auto CodeGenVisitor::handle(const Ident& node) -> std::string_view
{
	std::string_view name = node.get_value();
	return name;
}

auto CodeGenVisitor::handle(const ParamList& node)
		-> std::pair<std::vector<std::string_view>, std::vector<llvm::Type*>>
{
	std::vector<std::string_view> names;
	std::vector<llvm::Type*> type_list;

	type_list.reserve(node.get_params().size());
	names.reserve(node.get_params().size());

	for (const auto& param : node)
	{
		assert(param != nullptr);
		auto [name, type] = handle(*param);
		names.push_back(name);
		type_list.push_back(type);
	}

	return { names, type_list };
}

auto CodeGenVisitor::create_basic_block(const Block& node, llvm::Function* func,
										std::string_view block_name,
										std::span<std::string_view> param_names)
	-> llvm::BasicBlock*
{
	auto basic_block =
		llvm::BasicBlock::Create(get_llvm_context(), block_name.data(), func);

	llvm::Function::arg_iterator args_itr = func->arg_begin();
	std::vector<llvm::Value*> arg_values(func->arg_size());
	assert(arg_values.size() == param_names.size());

	for (auto& arg : arg_values)
	{
		arg = &*args_itr;
		++args_itr;
	}
	assert(args_itr == func->arg_end());
	
	get_builder().SetInsertPoint(basic_block);

	// 局部符号表
	LocalSymbolTable table(func, get_global_table());

	for (std::size_t i = 0; i < param_names.size(); ++i)
	{
		llvm::Type* type = arg_values[i]->getType();
		llvm::AllocaInst* alloca = get_builder().CreateAlloca(type);
		get_builder().CreateStore(arg_values[i], alloca);
		auto entry = std::make_shared<SymbolEntry>(alloca);
		table.insert(param_names[i], entry);
	}

	handle(node.get_block_item_list(), table);

	return basic_block;
}

void CodeGenVisitor::handle(const Block& node, LocalSymbolTable& upper_table)
{
	LocalSymbolTable table { &upper_table };
	handle(node.get_block_item_list(), table);
}

void CodeGenVisitor::handle(const BlockItemList& node, LocalSymbolTable& table)
{
	
	for (const auto& block_item : node)
	{
		assert(block_item != nullptr);
		handle(*block_item, table);
	}
	
}

void CodeGenVisitor::handle(const BlockItem& node, LocalSymbolTable& table)
{
	
	if (node.has_decl())
		handle(node.get_decl(), table);
	else if (node.has_stmt())
		handle(node.get_stmt(), table);
	
}

void CodeGenVisitor::handle(const SimpleStmt& node, LocalSymbolTable& table)
{
	
	switch (node.get_type())
	{
	case Stmt::assign:
	{
		auto left_entry = handle(node.get_lval(), table);
		if (!left_entry)
		{
			get_logger().info("Error happens in Lval");
			return;
		}

		llvm::Value* right_value = handle(node.get_expr(), table);

		if (left_entry->type != SymbolEntry::alloca_value) [[unlikely]]
		{
			report_in_ast(node, Location::DiagKind::dk_error,
						  "An eval value cannot be assigned");
			break;
		}

		if (left_entry == nullptr) [[unlikely]]
		{
			get_logger().info("User Error occured in Stmt::assign left value");
			break;
		}
		if (right_value == nullptr) [[unlikely]]
		{
			get_logger().info("User Error occured in Stmt::assign right value");
			break;
		}

		get_builder().CreateStore(right_value, left_entry->alloca);
		
		break;
	}
	case Stmt::expression:
	{
		if (!node.has_expr())
		{
			get_logger().debug("Empty Stmt expression");
			break;
		}

		auto value = handle(node.get_expr(), table);
		if (value == nullptr)
		{
			get_logger().info("User Error occured in Stmt::expression");
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
			get_logger().debug("Empty return statement");
			get_builder().CreateRetVoid();
			break;
		}
		auto value = handle(node.get_expr(), table);
		if (value == nullptr)
		{
			get_logger().info("User Error occured in Stmt::func_return");
		}
		get_builder().CreateRet(value);
		break;
	}
	case Stmt::if_stmt:
	{
		handle(node.get_select_stmt(), table);
		break;
	}
	case Stmt::while_stmt:
	{
		llvm::BasicBlock* cond = llvm::BasicBlock::Create(
			get_module()->getContext(), "", table.get_func());
		llvm::BasicBlock* body = llvm::BasicBlock::Create(
			get_module()->getContext(), "", table.get_func());
		llvm::BasicBlock* end = llvm::BasicBlock::Create(
			get_module()->getContext(), "", table.get_func());

		get_builder().CreateBr(cond);
		// 条件判断 begin
		get_builder().SetInsertPoint(cond);
		auto value = handle(node.get_expr(), table);
		get_builder().CreateCondBr(value, body, end);
		// 条件判断 end
		// 循环体
		get_builder().SetInsertPoint(body);
		handle(node.get_stmt(), table);
		get_builder().CreateBr(cond);
		// 循环体 end
		get_builder().SetInsertPoint(end);
		break;
	}
	default:
		get_logger().error("Unkown StmtType");
		std::abort();
	}
	
	
}

void CodeGenVisitor::handle(const SelectStmt& node, LocalSymbolTable& table)
{
	
	llvm::BasicBlock* if_then = llvm::BasicBlock::Create(
		get_module()->getContext(), "", table.get_func());
	llvm::BasicBlock* if_end = llvm::BasicBlock::Create(
		get_module()->getContext(), "", table.get_func());

	auto cmp = handle(node.get_expr(), table);
	if (!get_cvt_helper().convert_to_bool(cmp->getType()))
	{
		report_in_ast(node, Location::DiagKind::dk_error, "Cannot convert to bool");
		
		return;
	}

	if (!node.has_else_stmt())
	{
		get_builder().CreateCondBr(cmp, if_then, if_end);
		get_builder().SetInsertPoint(if_then);
		handle(node.get_if_stmt(), table);
		get_builder().CreateBr(if_end);
	}
	else
	{
		llvm::BasicBlock* if_else = llvm::BasicBlock::Create(
			get_module()->getContext(), "", table.get_func());
		get_builder().CreateCondBr(cmp, if_then, if_else);
		get_builder().SetInsertPoint(if_then);
		handle(node.get_if_stmt(), table);
		get_builder().SetInsertPoint(if_else);
		handle(node.get_else_stmt(), table);
		get_builder().CreateBr(if_end);
	}

	
}

auto CodeGenVisitor::handle(const Expr& node, LocalSymbolTable& table)
	-> llvm::Value*
{
	
	auto ret = handle(node.get_low_expr(), table);
	

	return ret;
}

auto CodeGenVisitor::handle(const PrimaryExpr& node, LocalSymbolTable& table)
	-> llvm::Value*
{
	

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
			get_logger().info("User Error Occured in LVal");
		}
		else
		{
			result = entry->type == SymbolEntry::eval_value ?
				entry->value :
				get_builder().CreateLoad(entry->alloca->getAllocatedType(), entry->alloca);
		}
	}
	else if (node.has_number())
	{
		result = handle(node.get_number());
	}
	
	if (result == nullptr)
	{
		get_logger().info("User Error Occured in PrimaryExpr");
	}

	
	return result;
}

auto CodeGenVisitor::handle(const UnaryExpr& node, LocalSymbolTable& table)
	-> llvm::Value*
{
	static auto handle_func = [this](const UnaryExpr& node) -> llvm::Function*
	{
		auto func_name = handle(node.get_ident());
		auto entry = get_global_table()->find(func_name);
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

		get_builder().CreateCall(func, passing_list);
		break;
	}
	case UnaryExpr::call: {
		auto func = handle_func(node);
		if (!func)
			return nullptr;

		get_builder().CreateCall(func);
		break;
	}
	default:
		assert(false && "UnaryExpr has an unkown type");
	}

	

	return result;
}

auto CodeGenVisitor::handle(const PassingParams& node, LocalSymbolTable& table)
		-> std::vector<llvm::Value*>
{
	
	std::vector<llvm::Value*> result;
	result.reserve(node.size());
	result.push_back(handle(node.get_expr(), table));
	handle(node.get_expr_list(), table, result);

	

	return result;
}

auto CodeGenVisitor::handle(const ExprList& node, LocalSymbolTable& table,
							std::vector<llvm::Value*>& list) -> bool
{
	
	for (const auto& expr: node)
	{
		auto ret = handle(*expr, table);
		if (!ret)
			return false;
		list.push_back(ret);
	}

	

	return true;
}

auto CodeGenVisitor::handle(const Number& node) -> llvm::Value*
{
	

	llvm::Value* result = llvm::ConstantInt::get(get_type_mgr().get_signed_int(),
											 node.get_int_literal());
	
	return result;
}

void CodeGenVisitor::handle(const Decl& node, LocalSymbolTable& table)
{
	

	if (node.has_const_decl())
		handle(node.get_const_decl(), table);
	else if (node.has_var_decl())
		handle(node.get_var_decl(), table);
	else
		assert(false && "Unkown type in Decl");

	
}

void CodeGenVisitor::handle(const ConstDecl& node, LocalSymbolTable& table)
{
	

	llvm::Type* type = handle(node.get_scalar_type());
	handle(node.get_first_const_def(), type, table);
	handle(node.get_const_def_list(), type, table);

	
}

void CodeGenVisitor::handle(const ConstDef& node, llvm::Type* type,
							LocalSymbolTable& table)
{
	

	auto name_str = handle(node.get_ident());

	llvm::Value* right_value = handle(node.get_const_init_val(), table);
	
	auto ret = get_cvt_helper().value_conversion(
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
	
	
}

void CodeGenVisitor::handle(const ConstDefList& node, llvm::Type* type,
							LocalSymbolTable& table)
{
	
	for (const auto& const_def_ptr : node )
	{
		handle(*const_def_ptr, type, table);
	}
	
}

auto CodeGenVisitor::handle(const ConstInitVal& node, LocalSymbolTable& table)
	-> llvm::Value*
{
	
	auto ret = handle(node.get_const_expr(), table);
	
	return ret;
}

auto CodeGenVisitor::handle(const ConstExpr& node, LocalSymbolTable& table)
	-> llvm::Value*
{
	
	auto result = handle(node.get_expr(), table);
	

	return result;
}

auto CodeGenVisitor::handle(const LVal& node, LocalSymbolTable& table)
	-> std::shared_ptr<SymbolEntry>
{
	
	auto name = handle(node.get_id());
	auto entry = table.lookup(name);
	if (entry == nullptr)
	{
		report_in_ast(node, Location::dk_error,
					std::format("Variable {} not defined", name));
		return nullptr;
	}

	
	
	return entry;
}

auto CodeGenVisitor::unary_operate(const UnaryOp& op, llvm::Value* operand)
	-> llvm::Value*
{
	llvm::Value* result = nullptr;
	llvm::Type* type = operand->getType();

	if (!type->isIntegerTy() && !type->isFloatingPointTy())
	{
		get_logger().error("Expected type in UnaryOp");
		abort();
	}

	switch (op.get_type())
	{
	case UnaryOp::op_add:
		result = operand;
		break;
	case UnaryOp::op_sub:
		if (type->isIntegerTy())
			result = get_builder().CreateNeg(operand);
		else 
			result = get_builder().CreateFNeg(operand);
		break;
	/// c语言not操作将操作数转换为int类型
	case UnaryOp::op_not:
	{
		llvm::Value* zero = llvm::ConstantInt::get(type, 0);
		llvm::Value* is_nonzero = nullptr;
		if (type->isIntegerTy())
		{
			is_nonzero = get_builder().CreateICmpNE(operand, zero);
		}
		else
		{
			is_nonzero = get_builder().CreateFCmpUNE(operand, zero);
		}
		llvm::Value* int_value =
			get_builder().CreateZExt(is_nonzero, get_type_mgr().get_signed_int());

		result = get_builder().CreateNot(int_value);
		break;
	}
	default:
		get_logger().error("Unkown operation: {}, category: {}",
						static_cast<int>(op.get_type()), op.get_type_str());
		abort();
	}

	return result;
}

auto CodeGenVisitor::handle(const Param& node) -> std::pair<std::string_view, llvm::Type*>
{
	auto name = handle(node.get_ident());
	auto type = handle(node.get_type());

	return { name, type };
}

template <typename TBinaryExpr>
	requires std::derived_from<TBinaryExpr, BinaryExprBase>
auto CodeGenVisitor::handle(const TBinaryExpr& node, LocalSymbolTable& table)
	-> llvm::Value*
{
	

	llvm::Value* result = nullptr;

	if (node.has_higher_expr())
	{
		result = handle(node.get_higher_expr(), table);
	}
	else if (node.has_combined_expr())
	{
		auto [self_expr_ref, op, higher_expr_ref ] = node.get_combined_expr();
		auto left = handle(self_expr_ref.get(), table);
		if (!left)
		{
			get_logger().info("Error happens in {}", self_expr_ref.get().get_kind_str());
			
			return nullptr;
		}
		auto right = handle(higher_expr_ref.get(), table);
		if (!right)
		{
			get_logger().info("Error happens in {}", higher_expr_ref.get().get_kind_str());
			return nullptr;
		}

		result = binary_operate(left, op, right);
	}
	else
	{
		assert(false && "Variant has an unkown type");
	}
	

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
		m_success = false;
		report_in_ast(node, Location::dk_warning, result.ec.message());
		return nullptr;
	default:
		assert(false);
	}
}

auto CodeGenVisitor::binary_operate(llvm::Value* left, const Operator& op,
									llvm::Value* right) -> llvm::Value*
{
	get_logger().debug("{} [{}] Begin:", op.get_kind_str(), op.get_type_str());

	llvm::Value* result = nullptr;
	assert(left && right);
	assert(left->getType() == right->getType());
	
	switch(op.get_type())
	{
	case Operator::op_add:
		result = get_builder().CreateAdd(left, right);
		break;
	case Operator::op_sub:
		result = get_builder().CreateSub(left, right);
		break;
	case Operator::op_mul:
		result = get_builder().CreateMul(left, right);
		break;
	case Operator::op_div:
		result = get_builder().CreateSDiv(left, right);
		break;
	case Operator::op_mod:
		result = get_builder().CreateSRem(left, right);
		break;
	case Operator::op_lt:
		result = get_builder().CreateICmpSLT(left, right);
		break;
	case Operator::op_le:
		result = get_builder().CreateICmpSLE(left, right);
		break;
	case Operator::op_gt:
		result = get_builder().CreateICmpSGT(left, right);
		break;
	case Operator::op_ge:
		result = get_builder().CreateICmpSGE(left, right);
		break;
	case Operator::op_eq:
		result = get_builder().CreateICmpEQ(left, right);
		break;
	case Operator::op_ne:
		result = get_builder().CreateICmpNE(left, right);
		break;
	case Operator::op_land:
		left = get_builder().CreateTrunc(left, llvm::Type::getInt1Ty(get_module()->getContext()));
		right = get_builder().CreateTrunc(right, llvm::Type::getInt1Ty(get_module()->getContext()));
		result = get_builder().CreateLogicalAnd(left, right);
		break;
	case Operator::op_lor:
		left = get_builder().CreateTrunc(left, llvm::Type::getInt1Ty(get_module()->getContext()));
		right = get_builder().CreateTrunc(right, llvm::Type::getInt1Ty(get_module()->getContext()));
		result = get_builder().CreateLogicalOr(left, right);
		break;
	default:
		//在二元运算符中
		assert(false && "Unprocessed binary operate");
	}

	assert(result != nullptr);
	get_builder().CreateZExt(result, get_type_mgr().get_signed_int());

	get_logger().debug("{} [{}] End", op.get_kind_str(), op.get_type_str());

	return result;
}

void CodeGenVisitor::handle(const VarDecl& node, LocalSymbolTable& table)
{
	

	auto* type = handle(node.get_scalar_type());
	handle(node.get_var_def(), type, table);
	handle(node.get_var_def_list(), type, table);

	
}

void CodeGenVisitor::handle(const VarDef& node, llvm::Type* type,
							LocalSymbolTable& table)
{
	

	auto name_str = handle(node.get_ident());

	// 创建一个新的局部变量，分配内存
	llvm::AllocaInst* alloca_inst =
		new llvm::AllocaInst(type, 0, name_str, get_builder().GetInsertBlock());

	if (node.is_initialized())
	{

		ConversionResult cvt_result;
		auto right_value = handle(node.get_init_val(), table);
		// 判断隐式类型转换是否合法
		cvt_result =
			get_cvt_helper().value_conversion(type, right_value->getType());
		auto left_type = report_conversion_result(cvt_result, node);
		if (left_type == nullptr)
			return;

		get_builder().CreateStore(right_value, alloca_inst);
	}
	// 在符号表中添加对应条目
	auto entry = std::make_shared<SymbolEntry>(alloca_inst);
	table.insert(name_str, entry);

	
}

void CodeGenVisitor::handle(const VarDefList& node, llvm::Type* type,
							LocalSymbolTable& table)
{
	
	for (const auto& ptr : node)
	{
		handle(*ptr, type, table);
	}

	
}

auto CodeGenVisitor::handle(const InitVal& node, LocalSymbolTable& table)
	-> llvm::Value*
{
	
	auto result = handle(node.get_expr(), table);
	return result;
}

void CodeGenVisitor::report_in_ast(const BaseAST& node, Location::DiagKind kind,
								   std::string_view msg)
{
	if (kind == Location::DiagKind::dk_error)
		m_success = false;
	node.report(kind, msg, &get_src_mgr());
}

}	//namespace toycc


