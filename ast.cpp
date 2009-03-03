#include "ast.h"
#include <iostream>
#include <sstream>

using namespace std;

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
		cout << " ";
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

/// ArgAST
ArgAST::ArgAST(int _n, TypeAST _type) : n(_n), type(_type)
{
}

TypeAST ArgAST::InputType(int index)
{
	assert(false);
	return TYPE_NULL;
}

TypeAST ArgAST::OutputType(int index)
{
	assert(index == 0);
	return type;
}

int ArgAST::InputSize()
{
	return 1;
}

int ArgAST::OutputSize()
{
	return 1;
}

void ArgAST::Print()
{
	cout << "arg" << n;
}

void ArgAST::DoCompile(IRBuilder<> builder)
{
	BasicBlock *bb = builder.GetInsertBlock();
	Function *f = bb->getParent();
	Function::arg_iterator arg_it = f->arg_begin();
	for(int i = 0; i < n; i++,arg_it++);
	SetValue(0, arg_it);
}

/// OutputIndexAST
OutputIndexAST::OutputIndexAST(AST *_ast, int _index)
	: ast(_ast), index(_index), compiled(false)
{
}

TypeAST OutputIndexAST::OutputType()
{
	return ast->OutputType(this->index);
}

void OutputIndexAST::Print()
{
	if(ast->OutputSize() == 1)
		ast->Print();
	else
	{
		cout << "[";
		ast->Print();
		cout << "]:" << index;
	}
}

void OutputIndexAST::Compile(IRBuilder<> builder)
{
	if(compiled)
		return;

	value = ast->GetValue(index, builder);
	compiled = true;
}

Value *OutputIndexAST::GetValue(IRBuilder<> builder)
{
	Compile(builder);
	return value;
}

