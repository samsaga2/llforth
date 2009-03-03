#pragma once

#include <vector>
#include "ast.h"
#include "coreast.h"

class InferenceStack
{
public:
	OutputList stack;
	ArgumentsList args;

	void Clear();
	OutputIndexAST *Pop(TypeAST type);
	void Push(AST *ast);
	void Print();
};

