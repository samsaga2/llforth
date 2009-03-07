#pragma once

#include <llvm/Type.h>

using namespace llvm;

enum TypeAST
{
	TYPE_NULL,
	TYPE_INT32,
	TYPE_STRING,
	TYPE_FLOAT
};

extern void PrintType(TypeAST t);
extern const Type *ConvertType(TypeAST t);

