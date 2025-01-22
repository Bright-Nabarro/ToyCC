#pragma once
#include <unordered_map>
#include <string_view>
#include <llvm/IR/Value.h>

class SymbolTable
{
public:
	
private:
	std::unordered_map<std::string_view, llvm::Value*> m_table;
};

