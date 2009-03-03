#include "coreast.h"
#include <iostream>

using namespace std;

/// IntegerAST
IntegerAST::IntegerAST(int _integer) : integer(_integer)
{
}

TypeAST IntegerAST::InputType(int index)
{
	assert(false);
	return TYPE_NULL;
}

TypeAST IntegerAST::OutputType(int index)
{
	assert(index == 0);
	return TYPE_INT32;
}

int IntegerAST::InputSize()
{
	return 0;
}

int IntegerAST::OutputSize()
{
	return 1;
}

void IntegerAST::Print()
{
	cout << integer;
}

void IntegerAST::DoCompile(IRBuilder<> builder)
{
	SetValue(0, ConstantInt::get(APInt(32, integer)));
}

/// StringAST
StringAST::StringAST(const string &_str) : str(_str)
{
}

TypeAST StringAST::InputType(int index)
{
	assert(false);
	return TYPE_NULL;
}

TypeAST StringAST::OutputType(int index)
{
	assert(index == 0);
	return TYPE_STRING;
}

int StringAST::InputSize()
{
	return 0;
}

int StringAST::OutputSize()
{
	return 1;
}

void StringAST::Print()
{
	cout << "\"" << str << "\"";
}

void StringAST::DoCompile(IRBuilder<> builder)
{
	SetValue(0, builder.CreateGlobalStringPtr(str.c_str()));
}

/// CallAST
CallAST::CallAST(FunctionBaseAST *_function, BodyAST *_args)
	: function(_function), args(_args)
{
}

TypeAST CallAST::InputType(int index)
{
	return function->InputType(index);
}

TypeAST CallAST::OutputType(int index)
{
	return function->OutputType(index);
}

int CallAST::InputSize()
{
	return function->InputSize();
}

int CallAST::OutputSize()
{
	return function->OutputSize(); 
}

void CallAST::Print() 
{
	cout << function->Name(); 
}

void CallAST::DoCompile(IRBuilder<> builder)
{
	Function *compiled_function = function->CompiledFunction();
	vector<Value *> func_args;
	for(BodyAST::iterator it = args->begin(); it != args->end(); it++)
	{
		Value *value = (*it)->GetValue(0, builder);
		func_args.push_back(value);
	}

	Value *result = builder.CreateCall<vector<Value *>::iterator>(compiled_function, func_args.begin(), func_args.end());

	switch(function->OutputSize())
	{
		case 0: break; // nothing
		case 1: SetValue(0, result); break;
		default:
			for(int i = 0; i < function->OutputSize(); i++)
			{
				Value *value = builder.CreateExtractValue(result, i);
				SetValue(i, value);
			}
			break;
	}
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
	: ast(_ast), index(_index)
{
}

TypeAST OutputIndexAST::InputType(int index)
{
	assert(false);
	return TYPE_NULL;
}

TypeAST OutputIndexAST::OutputType(int index)
{
	assert(index == 0);
	return ast->OutputType(this->index);
}

int OutputIndexAST::InputSize()
{
	return 0;
}

int OutputIndexAST::OutputSize()
{
	return 1;
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

void OutputIndexAST::DoCompile(IRBuilder<> builder)
{
	SetValue(0, ast->GetValue(index, builder));
}

/// DupAST
DupAST::DupAST(OutputIndexAST *_arg1) : arg1(_arg1)
{
}

TypeAST DupAST::InputType(int index)
{
	assert(index == 0);
	return arg1->OutputType(0);
}

TypeAST DupAST::OutputType(int index)
{
	assert(index == 0 || index == 1);
	return arg1->OutputType(0);
}

int DupAST::InputSize()
{
	return 1;
}

int DupAST::OutputSize()
{
	return 2;
}

void DupAST::Print()
{
	cout << "dup";
}

void DupAST::DoCompile(IRBuilder<> builder)
{
	Value *v = arg1->GetValue(0, builder);
	SetValue(0, v);
	SetValue(1, v);
}

/// MultAST
MultAST::MultAST(OutputIndexAST *_arg1, OutputIndexAST *_arg2) : arg1(_arg1), arg2(_arg2)
{
}

TypeAST MultAST::InputType(int index)
{
	assert(index == 0 || index == 1);
	return TYPE_INT32;
}

TypeAST MultAST::OutputType(int index)
{
	assert(index == 0);
	return TYPE_INT32;
}

int MultAST::InputSize()
{
	return 2;
}

int MultAST::OutputSize()
{
	return 1;
}

void MultAST::Print()
{
	cout << "*";
}

void MultAST::DoCompile(IRBuilder<> builder)
{
	SetValue(0, builder.CreateMul(arg1->GetValue(0, builder), arg2->GetValue(0, builder)));
}

/// AddAST
AddAST::AddAST(OutputIndexAST *_arg1, OutputIndexAST *_arg2) : arg1(_arg1), arg2(_arg2)
{
}

TypeAST AddAST::InputType(int index)
{
	assert(index == 0 || index == 1);
	return TYPE_INT32;
}

TypeAST AddAST::OutputType(int index)
{
	assert(index == 0);
	return TYPE_INT32;
}

int AddAST::InputSize()
{
	return 2;
}

int AddAST::OutputSize()
{
	return 1;
}

void AddAST::Print()
{
	cout << "+";
}

void AddAST::DoCompile(IRBuilder<> builder)
{
	Value *v1 = arg1->GetValue(0, builder);
	Value *v2 = arg2->GetValue(0, builder);
	SetValue(0, builder.CreateAdd(v1, v2));
}

/// SwapAST
SwapAST::SwapAST(OutputIndexAST *_arg1, OutputIndexAST *_arg2) : arg1(_arg2), arg2(_arg1)
{
}

TypeAST SwapAST::InputType(int index)
{
	assert(index == 0 || index == 1);
	if(index == 0)
		return arg1->OutputType(0);
	else
		return arg2->OutputType(0);
}

TypeAST SwapAST::OutputType(int index)
{
	assert(index == 0 || index == 1);

	if(index == 0)
		return arg1->OutputType(0);
	else
		return arg2->OutputType(0);
}

int SwapAST::InputSize()
{
	return 2;
}

int SwapAST::OutputSize()
{
	return 2;
}

void SwapAST::Print()
{
	arg2->Print();
	cout << " ";
	arg1->Print();
}

void SwapAST::DoCompile(IRBuilder<> builder)
{
	SetValue(0, arg1->GetValue(0, builder));
	SetValue(1, arg2->GetValue(0, builder));
}

// OverAST
OverAST::OverAST(OutputIndexAST *_arg1, OutputIndexAST *_arg2) : arg1(_arg1), arg2(_arg2)
{
}

TypeAST OverAST::InputType(int index)
{
	assert(index == 0 || index == 1);
	if(index == 0)
		return arg1->OutputType(0);
	else
		return arg2->OutputType(0);
}

TypeAST OverAST::OutputType(int index)
{
	assert(index >= 0 && index <= 2);
	if(index == 0 || index == 2)
		return arg1->OutputType(0);
	else
		return arg2->OutputType(0);
}

int OverAST::InputSize()
{
	return 2;
}

int OverAST::OutputSize()
{
	return 3;
}

void OverAST::Print()
{
	arg1->Print();
	cout << " ";
	arg2->Print();
	cout << " ";
	arg1->Print();
}

void OverAST::DoCompile(IRBuilder<> builder)
{
	SetValue(0, arg1->GetValue(0, builder));
	SetValue(1, arg2->GetValue(0, builder));
	SetValue(2, arg1->GetValue(0, builder));
}

/// RotAST
RotAST::RotAST(OutputIndexAST *_arg1, OutputIndexAST *_arg2, OutputIndexAST *_arg3)
	: arg1(_arg1), arg2(_arg2), arg3(_arg3)
{
}

TypeAST RotAST::InputType(int index)
{
	assert(index >= 0 && index <= 2);
	if(index == 0)
		return arg1->OutputType(0);
	else if(index == 1)
		return arg2->OutputType(0);
	else
		return arg3->OutputType(0);
}

TypeAST RotAST::OutputType(int index)
{
	assert(index >= 0 && index <= 2);
	if(index == 0)
		return arg2->OutputType(0);
	else if(index == 1)
		return arg3->OutputType(0);
	else
		return arg1->OutputType(0);
}

int RotAST::InputSize()
{
	return 3;
}

int RotAST::OutputSize()
{
	return 3;
}

void RotAST::Print()
{
	arg2->Print();
	cout << " ";
	arg3->Print();
	cout << " ";
	arg1->Print();
}

void RotAST::DoCompile(IRBuilder<> builder)
{
	arg1->Compile(builder);
	arg2->Compile(builder);
	SetValue(0, arg1->GetValue(0, builder));
	SetValue(2, arg2->GetValue(0, builder));
	SetValue(1, arg3->GetValue(0, builder));
}
//
/// DropAST
DropAST::DropAST(OutputIndexAST *_arg1) : arg1(_arg1)
{
}

TypeAST DropAST::InputType(int index)
{
	assert(index == 0);
	return TYPE_ANY;
}

TypeAST DropAST::OutputType(int index)
{
	assert(false);
	return TYPE_NULL;
}

int DropAST::InputSize()
{
	return 1;
}

int DropAST::OutputSize()
{
	return 0;
}

void DropAST::Print()
{
	cout << "drop";
}

void DropAST::DoCompile(IRBuilder<> builder)
{
	arg1->Compile(builder);
}

