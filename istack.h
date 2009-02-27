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
		Counter(AST *_ast) : ast(_ast), index(0) { }
	};

	std::vector<Counter*> stack;
	BodyAST args;

	void Clear();
	OutputIndexAST *Pop();
	BodyAST *Pop(int size);
	void Push(AST *ast);
};

