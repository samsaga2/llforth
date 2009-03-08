#pragma once

#include <llvm/Type.h>

using namespace llvm;

enum TypeAST
{
	TYPE_INT32,
	TYPE_FLOAT
};

extern void PrintType(TypeAST t);
extern const Type *ConvertType(TypeAST t);
extern TypeAST ConvertTypeAST(const std::string& type);

