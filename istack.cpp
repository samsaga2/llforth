#include "istack.h"

void InferenceStack::Clear()
{
	stack.clear();
	args.clear();
}

OutputIndexAST *InferenceStack::Pop()
{
	if(stack.size() == 0)
	{
		AST *arg = new ArgAST(args.size());
		args.push_back(arg);
		Push(arg);
	}

	assert(stack.size() != 0);
	Counter *counter = stack.back();
	OutputIndexAST *stack_index = new OutputIndexAST(counter->ast, counter->index);

	counter->index++;
	if(counter->index == counter->ast->OutputSize())
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

