#pragma once

#include "ast.h"
#include <vector>

class InferenceStack
{
public:
	class Counter
	{
	public:
		AST *ast;
		int index;
		Counter(AST *_ast);
	};

	std::list<Counter*> stack;
	BodyAST args;

	void Clear();
	OutputIndexAST *Pop(TypeAST type);
	void Push(AST *ast);
	void Print();
};

