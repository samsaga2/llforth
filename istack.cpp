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

OutputIndexAST *InferenceStack::Pop()
{
	while(stack.back()->index == 0)
	{
		delete stack.back();
		stack.pop_back();
	}

	if(stack.size() == 0)
	{
		AST *arg = new ArgAST(args.size());
		args.push_back(arg);
		Push(arg);
	}

	assert(stack.size() != 0);
	Counter *counter = stack.back();
	OutputIndexAST *stack_index = new OutputIndexAST(counter->ast, --counter->index);

	if(counter->index == 0)
	{
		delete counter;
		stack.pop_back();
	}

	return stack_index;
}

BodyAST *InferenceStack::Pop(int size)
{
	BodyAST *body = new BodyAST();
	while(size > 0)
	{
		body->push_back(Pop());
		size--;
	}
}

void InferenceStack::Push(AST *ast)
{
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

