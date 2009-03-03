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
CallAST::CallAST(FunctionBaseAST *_function, OutputList *_args)
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
	for(OutputList::iterator it = args->begin(); it != args->end(); it++)
	{
		Value *value = (*it)->GetValue(builder);
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

/// DupAST
DupAST::DupAST(OutputIndexAST *_arg1) : arg1(_arg1)
{
}

TypeAST DupAST::InputType(int index)
{
	assert(index == 0);
	return arg1->OutputType();
}

TypeAST DupAST::OutputType(int index)
{
	assert(index == 0 || index == 1);
	return arg1->OutputType();
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
	Value *v = arg1->GetValue(builder);
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
	SetValue(0, builder.CreateMul(arg1->GetValue(builder), arg2->GetValue(builder)));
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
	Value *v1 = arg1->GetValue(builder);
	Value *v2 = arg2->GetValue(builder);
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
		return arg1->OutputType();
	else
		return arg2->OutputType();
}

TypeAST SwapAST::OutputType(int index)
{
	assert(index == 0 || index == 1);

	if(index == 0)
		return arg1->OutputType();
	else
		return arg2->OutputType();
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
	SetValue(0, arg1->GetValue(builder));
	SetValue(1, arg2->GetValue(builder));
}

// OverAST
OverAST::OverAST(OutputIndexAST *_arg1, OutputIndexAST *_arg2) : arg1(_arg1), arg2(_arg2)
{
}

TypeAST OverAST::InputType(int index)
{
	assert(index == 0 || index == 1);
	if(index == 0)
		return arg1->OutputType();
	else
		return arg2->OutputType();
}

TypeAST OverAST::OutputType(int index)
{
	assert(index >= 0 && index <= 2);
	if(index == 0 || index == 2)
		return arg1->OutputType();
	else
		return arg2->OutputType();
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
	cout << "over";
}

void OverAST::DoCompile(IRBuilder<> builder)
{
	SetValue(0, arg1->GetValue(builder));
	SetValue(1, arg2->GetValue(builder));
	SetValue(2, arg1->GetValue(builder));
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
		return arg1->OutputType();
	else if(index == 1)
		return arg2->OutputType();
	else
		return arg3->OutputType();
}

TypeAST RotAST::OutputType(int index)
{
	assert(index >= 0 && index <= 2);
	if(index == 0)
		return arg2->OutputType();
	else if(index == 1)
		return arg3->OutputType();
	else
		return arg1->OutputType();
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
	cout << "rot";
}

void RotAST::DoCompile(IRBuilder<> builder)
{
	SetValue(0, arg1->GetValue(builder));
	SetValue(2, arg2->GetValue(builder));
	SetValue(1, arg3->GetValue(builder));
}

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

/// CastIntToStringAST
CastIntToStringAST::CastIntToStringAST(OutputIndexAST *_arg1) : arg1(_arg1)
{
}

TypeAST CastIntToStringAST::InputType(int index)
{
	assert(index == 0);
	return TYPE_INT32;
}

TypeAST CastIntToStringAST::OutputType(int index)
{
	assert(index == 0);
	return TYPE_STRING;
}

int CastIntToStringAST::InputSize()
{
	return 1;
}

int CastIntToStringAST::OutputSize()
{
	return 1;
}

void CastIntToStringAST::Print()
{
	cout << "i>s";
}

void CastIntToStringAST::DoCompile(IRBuilder<> builder)
{
	Value *value = arg1->GetValue(builder);
	Type *string_type = PointerType::getUnqual(Type::Int8Ty);
	SetValue(0, builder.CreateIntToPtr(value, string_type));
}

