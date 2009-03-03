#include "ast.h"
#include <iostream>
#include <sstream>

/// AST
AST::AST() : compiled(false)
{
}

void AST::Compile(IRBuilder<> builder)
{
	if(compiled)
		return;

	DoCompile(builder);
	compiled = true;
}

Value *AST::GetValue(int index, IRBuilder<> builder)
{
	Compile(builder);
	return values[index];
}

void AST::SetValue(int index, Value *value)
{
	if(values.size() <= index)
		values.resize(index + 1);
	values[index] = value;
}

/// BodyAST
int BodyAST::OutputSize()
{
	int output_size = 0;
	for(BodyAST::iterator it = this->begin(); it != this->end(); it++)
		output_size += (*it)->OutputSize();
	return output_size;
}

TypeAST BodyAST::OutputType(int index)
{
	int output_size = 0;
	size_t idx = 0;
	for(BodyAST::iterator it = this->begin(); it != this->end(); it++)
	{
		AST *ast = (*it);
		for(size_t i = 0; i < ast->OutputSize(); i++)
		{
			if(idx++ == index)
				return ast->OutputType(i);
		}
	}

	assert(false);
}

void BodyAST::Print()
{
	for(BodyAST::iterator it = this->begin(); it != this->end(); it++)
	{
		std::cout << " ";
		(*it)->Print();
	}
}

AST *BodyAST::operator[](size_t index)
{
	BodyAST::iterator it = this->begin();
	while(it != this->end() && index > 0)
	{
		index--;
		it++;
	}
	return (*it);
}

