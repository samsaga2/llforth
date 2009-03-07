#include "coreast.h"
#include <iostream>

using namespace std;

/// IntegerAST
IntegerAST::IntegerAST(int _number) : number(_number)
{
}

TypeAST IntegerAST::InputType(int index)
{
	assert(false);
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
	cout << number;
}

void IntegerAST::DoCompile(IRBuilder<> builder)
{
	SetValue(0, ConstantInt::get(APInt(32, number)));
}

/// FloatAST
FloatAST::FloatAST(float _number) : number(_number)
{
}

TypeAST FloatAST::InputType(int index)
{
	assert(false);
}

TypeAST FloatAST::OutputType(int index)
{
	assert(index == 0);
	return TYPE_FLOAT;
}

int FloatAST::InputSize()
{
	return 0;
}

int FloatAST::OutputSize()
{
	return 1;
}

void FloatAST::Print()
{
	cout << number;
}

void FloatAST::DoCompile(IRBuilder<> builder)
{
	SetValue(0, ConstantFP::get(APFloat(number)));
}

/// StringAST
StringAST::StringAST(const string &_str) : str(_str)
{
}

TypeAST StringAST::InputType(int index)
{
	assert(false);
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
	return TYPE_INT32;
}

TypeAST DupAST::OutputType(int index)
{
	assert(index == 0 || index == 1);
	return TYPE_INT32;
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

/// AddIntegerAST
AddIntegerAST::AddIntegerAST(OutputIndexAST *_arg1, OutputIndexAST *_arg2) : arg1(_arg1), arg2(_arg2)
{
}

TypeAST AddIntegerAST::InputType(int index)
{
	assert(index == 0 || index == 1);
	return TYPE_INT32;
}

TypeAST AddIntegerAST::OutputType(int index)
{
	assert(index == 0);
	return TYPE_INT32;
}

int AddIntegerAST::InputSize()
{
	return 2;
}

int AddIntegerAST::OutputSize()
{
	return 1;
}

void AddIntegerAST::Print()
{
	cout << "+";
}

void AddIntegerAST::DoCompile(IRBuilder<> builder)
{
	Value *v1 = arg1->GetValue(builder);
	Value *v2 = arg2->GetValue(builder);
	SetValue(0, builder.CreateAdd(v1, v2));
}

/// AddFloatAST
AddFloatAST::AddFloatAST(OutputIndexAST *_arg1, OutputIndexAST *_arg2) : arg1(_arg1), arg2(_arg2)
{
}

TypeAST AddFloatAST::InputType(int index)
{
	assert(index == 0 || index == 1);
	return TYPE_FLOAT;
}

TypeAST AddFloatAST::OutputType(int index)
{
	assert(index == 0);
	return TYPE_FLOAT;
}

int AddFloatAST::InputSize()
{
	return 2;
}

int AddFloatAST::OutputSize()
{
	return 1;
}

void AddFloatAST::Print()
{
	cout << "+";
}

void AddFloatAST::DoCompile(IRBuilder<> builder)
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
	return TYPE_INT32;
}

TypeAST SwapAST::OutputType(int index)
{
	assert(index == 0 || index == 1);
	return TYPE_INT32;
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
	return TYPE_INT32;
}

TypeAST OverAST::OutputType(int index)
{
	assert(index >= 0 && index <= 2);
	return TYPE_INT32;
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
	return TYPE_INT32;
}

TypeAST RotAST::OutputType(int index)
{
	assert(index >= 0 && index <= 2);
	return TYPE_INT32;
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
	return TYPE_INT32;
}

TypeAST DropAST::OutputType(int index)
{
	assert(false);
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

