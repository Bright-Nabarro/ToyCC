#pragma once
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Instructions.h>

class MockValue: public llvm::Value
{
public:
	MockValue(llvm::Type* type, llvm::Value::ValueTy vty = llvm::Value::ValueTy::InstructionVal,
			const llvm::Twine& name = ""):
		llvm::Value { type, vty }
	{
		if (!name.isTriviallyEmpty())
			setName(name);
	}
	
	// MOCK_METHOD(返回类型, 方法名, (参数列表), (限定符));
	MOCK_METHOD(llvm::Type*, getType, (), (const));
	MOCK_METHOD(llvm::Use*, getOperandList, (), ());
	MOCK_METHOD(const llvm::Use*, getOperandList, (), (const));
	MOCK_METHOD(void, replaceAllUseWith, (llvm::Value*), ());
	MOCK_METHOD(void, takeName, (llvm::Value*), ());
  	MOCK_METHOD(bool, hasNUses, (unsigned), (const));
 	MOCK_METHOD(bool, hasNUsesOrMore, (unsigned), (const));
  	MOCK_METHOD(bool, hasOneUse, (), (const));
};

class MockType: public llvm::Type
{
public:
	MockType(llvm::LLVMContext& context, llvm::Type::TypeID tid)
		: llvm::Type{context, tid}
	{
	}

	MOCK_METHOD(bool, isIntegerTy, (), (const));
    MOCK_METHOD(bool, isFloatingPointTy, (), (const));
    MOCK_METHOD(bool, isPointerTy, (), (const));
    MOCK_METHOD(bool, isStructTy, (), (const));
    MOCK_METHOD(bool, isArrayTy, (), (const));
    MOCK_METHOD(bool, isVoidTy, (), (const));
    MOCK_METHOD(bool, isFunctionTy, (), (const));
    MOCK_METHOD(unsigned, getIntegerBitWidth, (), (const));
    MOCK_METHOD(llvm::Type*, getPointerTo, (unsigned), (const));
    MOCK_METHOD(llvm::Type*, getArrayElementType, (), (const));
    MOCK_METHOD(unsigned, getArrayNumElements, (), (const));
    MOCK_METHOD(llvm::Type*, getStructElementType, (unsigned), (const));
    MOCK_METHOD(unsigned, getStructNumElements, (), (const));
    MOCK_METHOD(llvm::FunctionType*, getFunctionParamType, (unsigned), (const));
    MOCK_METHOD(unsigned, getFunctionNumParams, (), (const));
    MOCK_METHOD(llvm::Type*, getFunctionReturnType, (), (const));
    MOCK_METHOD(bool, isSized, (llvm::DataLayout*), (const));
};


