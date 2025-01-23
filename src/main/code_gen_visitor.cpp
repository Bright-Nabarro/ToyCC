#include "code_gen_visitor.hpp"

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

CodeGenVisitor::CodeGenVisitor(llvm::LLVMContext& context, llvm::SourceMgr& src_mgr, llvm::TargetMachine* tm):
	m_module { std::make_unique<llvm::Module>("toycc.expr", context) },
	m_builder { m_module->getContext() },
	m_type_mgr { std::make_shared<TypeMgr>(m_module->getContext(), tm) },
	m_src_mgr { src_mgr },
	m_target_machine { tm }
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

void CodeGenVisitor::handle(const CompUnit& node)
{
	//yq::debug("CompUnitBegin:");
	handle(node.get_func_def());
	//yq::debug("CompUnitEnd");
}

void CodeGenVisitor::handle(const FuncDef& node)
{
	//yq::debug("FuncDefBegin:");
	auto return_type = handle(node.get_type());
	auto func_name = handle(node.get_ident());
	auto param_types = handle(node.get_paramlist());

	auto func_type = llvm::FunctionType::get(return_type, param_types, false);

	//auto func =
	//	llvm::Function::Create(func_type, llvm::GlobalValue::ExternalLinkage,
	//						   func_name, m_module.get());

	auto func =
		llvm::Function::Create(func_type, llvm::GlobalValue::ExternalLinkage,
							   func_name, m_module.get());
	handle(node.get_block(), func, "entry");

	//yq::debug("FuncDefEnd");
}

auto CodeGenVisitor::handle(const BuiltinType& node) -> llvm::Type*
{
	//yq::debug("Type[{}]Begin: ", node.get_type_str());
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
		//yq::fatal(yq::loc(), "Unkown TypeEnum int toycc::Type when handling "
		//		 "toycc::Type to llvm::Type*");
		ret = nullptr;
		break;
	}
	//yq::debug("Type[{}]End", node.get_type_str());
	
	return ret;
}

auto CodeGenVisitor::handle(const ScalarType& node) -> llvm::Type*
{
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
		//yq::fatal(yq::loc(), "Unkown TypeEnum int toycc::Type when handling "
		//		 "toycc::Type to llvm::Type*");
		ret = nullptr;
		break;
	}

	return ret;
}


auto CodeGenVisitor::handle(const Ident& node) -> std::string_view
{
	//yq::debug("Ident[{}]Begin:", node.get_value());
	
	std::string_view name = node.get_value();

	//yq::debug("Ident[{}]End:", node.get_value());

	return name;
}

auto CodeGenVisitor::handle(const ParamList& node) -> std::vector<llvm::Type*>
{
	//yq::debug("ParamListBegin: ");
	std::vector<llvm::Type*> type_list;
	type_list.reserve(node.get_params().size());

	for (const auto& param : node)
	{
		assert(param != nullptr);
		type_list.push_back(handle(*param));
	}

	//yq::debug("ParamListEnd");

	return type_list;
}

auto CodeGenVisitor::handle(const Block& node, llvm::Function* func,
							std::string_view block_name) -> llvm::BasicBlock*
{
	//yq::debug("BlockBegin: ");

	auto basic_block =
		llvm::BasicBlock::Create(m_module->getContext(), block_name.data(), func);
	m_builder.SetInsertPoint(basic_block);

	//yq::debug("BlockEnd");

	return basic_block;
}

void CodeGenVisitor::handle(const BlockItemList& node)
{
	//yq::debug("BlockItemListBegin:" );
	for (const auto& block_item : node)
	{
		assert(block_item != nullptr);
		handle(*block_item);
	}
	//yq::debug("BlockItemListEnd");
}

void CodeGenVisitor::handle(const BlockItem& node)
{
	//yq::debug("BlockItemBegin: ");
	if (node.has_decl())
		handle(node.get_decl());
	else if (node.has_stmt())
		handle(node.get_stmt());
	//yq::debug("BlockItemEnd");
}

void CodeGenVisitor::handle(const Stmt& node)
{
	//yq::debug("StmtBegin:");
	auto value = handle(node.get_expr());
	assert(value != nullptr);
	
	m_builder.CreateRet(value);
	//yq::debug("StmtEnd");
}

auto CodeGenVisitor::handle(const Expr& node) -> llvm::Value*
{
	//yq::debug("ExprBegin:");
	auto ret = handle(node.get_low_expr());
	//yq::debug("ExprEnd");

	return ret;
}

auto CodeGenVisitor::handle(const PrimaryExpr& node) -> llvm::Value*
{
	//yq::debug("PrimaryExprBegin: ");

	llvm::Value* result = nullptr;
	if (node.has_expr())
	{
		result = handle(node.get_expr());	
	}
	else if (node.has_ident())
	{
		result = handle(node.get_lval());
	}
	else if (node.has_number())
	{
		result = handle(node.get_number());
	}

	//yq::debug("PrimaryExprEnd");
	return result;
}

auto CodeGenVisitor::handle(const UnaryExpr& node) -> llvm::Value*
{
	//yq::debug("UnaryExpr Begin:");

	llvm::Value* result = nullptr;
	if (node.has_unary_expr())
	{
		result = handle(node.get_unary_expr());
		result = unary_operate(node.get_unary_op(), result);
	}
	else if (node.has_primary_expr())
	{
		result = handle(node.get_primary_expr());
	}
	else
	{
		assert(false && "UnaryExpr has an unkown type in its variant");
	}

	//yq::debug("UnaryExpr End");
	return result;
}

auto CodeGenVisitor::handle(const Number& node) -> llvm::Value*
{
	//yq::debug("Number[{}] Begin: ", node.get_int_literal());

	llvm::Value* result = llvm::ConstantInt::get(m_type_mgr->get_signed_int(),
												 node.get_int_literal());

	//yq::debug("Number End");
	return result;
}

void CodeGenVisitor::handle(const Decl& node)
{
	handle(node.get_const_decl());
}

auto CodeGenVisitor::handle(const ConstDecl& node)
	-> std::vector<llvm::Value*>
{
	llvm::Type* type = handle(node.get_scalar_type());
	auto first_value = handle(node.get_first_const_def(), type);
	auto value_list = handle(node.get_const_def_list(), type);
	return value_list;
}

auto CodeGenVisitor::handle(const ConstDef& node, llvm::Type* type)
	-> llvm::Value*
{
	//yq::debug("{} Start", node.get_kind_str());

	auto name = handle(node.get_ident());

	llvm::Value* right_value = handle(node.get_const_int_val());
	llvm::Value* left_value = nullptr;

	auto ret =
		m_cvt_helper->value_conversion(std::make_shared<LLVMType>(type),
									   std::make_shared<LLVMType>(right_value->getType()));
	if (!ret)
	{
		//yq::error("{}", ret.error().message());
		return nullptr;
	}
	llvm::Type* left_type = get_llvm_type(*ret);
	
	left_value = right_value;
	left_value->mutateType(left_type);

	//yq::debug("{} End", node.get_kind_str());

	return left_value;
}

auto CodeGenVisitor::handle(const ConstDefList& node, llvm::Type* type)
	-> std::vector<llvm::Value*>
{
	std::vector<llvm::Value*> result;
	result.resize(node.size());

	for (const auto& const_def_ptr : node )
	{
		result.push_back(handle(*const_def_ptr, type));	
	}

	return result;
}

auto CodeGenVisitor::handle(const ConstInitVal& node) -> llvm::Value*
{
}

auto CodeGenVisitor::handle(const ConstExpr& node) -> llvm::Value*
{
	auto result = handle(node.get_expr());

	return result;
}

auto CodeGenVisitor::handle(const LVal& node) -> llvm::Value*
{
	
}


auto CodeGenVisitor::unary_operate(const UnaryOp& op, llvm::Value* operand)
	-> llvm::Value*
{
	//yq::debug("UnaryOp[{}] Begin:",op.get_type_str());

	llvm::Type* type = operand->getType();
	llvm::Value* result = nullptr;

	if (!type->isIntegerTy() && !type->isFloatingPointTy())
	{
		//yq::error("Expected type in UnaryOp");
		return nullptr;
	}

	switch(op.get_type())
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
	case UnaryOp::op_not: {
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
		llvm::Value* int_value = m_builder.CreateZExt(is_nonzero, m_type_mgr->get_signed_int());

		result = m_builder.CreateNot(int_value);
		break;
	}
	default:
		//yq::error("Unkown operation: {}, category: {}",
		//		  static_cast<int>(op.get_type()), op.get_type_str());
		result = nullptr;
	}
	
	//yq::debug("UnaryOp End");

	return result;
}

auto CodeGenVisitor::handle(const Param& node) -> llvm::Type*
{
	//yq::debug("ParamBegin: ");
	auto type = handle(node.get_type());
	handle(node.get_ident());
	//yq::debug("ParamEnd");

	return type;
}

template<typename TBinaryExpr>
requires std::derived_from<TBinaryExpr, BinaryExprBase>
auto CodeGenVisitor::handle(const TBinaryExpr& node) -> llvm::Value*
{
	//yq::debug("{} begin:", node.get_kind_str());

	llvm::Value* result = nullptr;

	if (node.has_higher_expr())
	{
		result = handle(node.get_higher_expr());
	}
	else if (node.has_combined_expr())
	{
		auto [self_expr_ref, op, higher_expr_ref ] = node.get_combined_expr();
		auto left = handle(self_expr_ref.get());
		auto right = handle(higher_expr_ref.get());
		
		result = binary_operate(left, op, right);
	}
	else
	{
		//yq::fatal("{}'s Variant has an unkown type", node.get_kind_str());
	}

	//yq::debug("{} end", node.get_kind_str());
	return result;
}


auto CodeGenVisitor::binary_operate(llvm::Value* left, const Operator& op,
									llvm::Value* right) -> llvm::Value*
{
	//yq::debug("{} [{}] Begin:", op.get_kind_str(), op.get_type_str());

	llvm::Value* result = nullptr;
	
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
	//default:
		//在二元运算符中
		//yq::fatal(yq::loc(), "Unprocessed binary operate");
	}

	assert(result != nullptr);
	m_builder.CreateZExt(result, m_type_mgr->get_signed_int());

	//yq::debug("{} [{}] End", op.get_kind_str(), op.get_type_str());

	return result;
}


}	//namespace toycc


