#include "istack.h"
#include <iostream>

InferenceStack::Counter::Counter(AST *_ast) : ast(_ast), index(_ast->OutputSize())
{
}

void InferenceStack::Clear()
{
	stack.clear();
	args.clear();
}

OutputIndexAST *InferenceStack::Pop(TypeAST type)
{
	while(stack.back()->index == 0)
	{
		delete stack.back();
		stack.pop_back();
	}

	if(stack.size() == 0)
	{
		AST *arg = new ArgAST(args.size(), type);
		args.push_back(arg);
		Push(arg);
	}

	assert(stack.size() != 0);
	Counter *counter = stack.back();
	OutputIndexAST *stack_index = new OutputIndexAST(counter->ast, --counter->index);

	assert(stack_index->OutputType(0) == type || type == TYPE_ANY);

	if(counter->index == 0)
	{
		delete counter;
		stack.pop_back();
	}

	return stack_index;
}

void InferenceStack::Push(AST *ast)
{
	if(ast->OutputSize() != 0)
		stack.push_back(new Counter(ast));
}

void InferenceStack::Print()
{
	std::cout << "<<stack<<" << std::endl;
	int i = 0;
	for(std::list<Counter *>::iterator it = stack.begin(); it != stack.end(); it++)
	{
		std::cout << "  " << i++ << ": idx:" << (*it)->index << " ";
		(*it)->ast->Print();
		std::cout << std::endl;
	}
	std::cout << ">>stack>>" << std::endl;
}

