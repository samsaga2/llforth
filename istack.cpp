#include "istack.h"
#include <iostream>

void InferenceStack::Clear()
{
	stack.clear();
	args.clear();
}

OutputIndexAST *InferenceStack::Pop(TypeAST type)
{
	if(stack.size() == 0)
	{
		ArgAST *arg = new ArgAST(args.size(), type);
		args.push_back(arg);
		Push(arg);
	}

	OutputIndexAST *value = stack.back();
	stack.pop_back();
	assert(value->OutputType(0) == type || type == TYPE_ANY);

	return value;
}

void InferenceStack::Push(AST *ast)
{
	for(size_t i = 0; i < ast->OutputSize(); i++)
		stack.push_back(new OutputIndexAST(ast, i));
}

void InferenceStack::Print()
{
	std::cout << "<<stack<<" << std::endl;
	for(OutputList::iterator it = stack.begin(); it != stack.end(); it++)
	{
		std::cout << "  ";
		(*it)->Print();
		std::cout << std::endl;
	}
	std::cout << ">>stack>>" << std::endl;
}

