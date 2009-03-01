#include "ast.h"
#include <iostream>
#include <sstream>

AST::AST() : compiled(false)
{
}

Value *AST::GetValue(int index, IRBuilder<> builder)
{
	if(!compiled)
	{
		Compile(builder);
		compiled = true;
	}
	return values[index];
}

void AST::SetValue(int index, Value *value)
{
	if(values.size() <= index)
		values.resize(index + 1);
	values[index] = value;
}

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
	std::cout << integer;
}

void IntegerAST::Compile(IRBuilder<> builder)
{
	SetValue(0, ConstantInt::get(APInt(32, integer)));
}

/// StringAST
StringAST::StringAST(const std::string &_str) : str(_str)
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
	std::cout << "\"" << str << "\"";
}

void StringAST::Compile(IRBuilder<> builder)
{
	SetValue(0, builder.CreateGlobalStringPtr(str.c_str()));
}

/// BodyAST
int BodyAST::OutputSize()
{
	int output_size = 0;
	for(BodyAST::iterator it = this->begin(); it != this->end(); it++)
		output_size += (*it)->OutputSize();
	return output_size;
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

/// FunctionAST
FunctionAST::FunctionAST(const std::string &_name, BodyAST *_body, BodyAST *_args)
	: name(_name), body(_body), args(_args)
{
}

const std::string FunctionAST::Name()
{
	return name;
}

TypeAST FunctionAST::InputType(int index)
{
	if(args->size() == 0)
		assert(false);
	else
		assert(index < args->size());

	return (*args)[index]->InputType(0);
}

TypeAST FunctionAST::OutputType(int index)
{
	if(body->size() == 0)
		assert(false);
	else
		assert(index < body->size());

	return (*body)[index]->OutputType(0);
}

int FunctionAST::InputSize()
{
	return args->OutputSize();
}

int FunctionAST::OutputSize()
{
	return body->OutputSize();
}

void PrintType(TypeAST t)
{
	switch(t)
	{
		case TYPE_NULL: std::cout << " *"; break;
		case TYPE_INT32: std::cout << " i"; break;
		case TYPE_STRING: std::cout << " s"; break;
	}
}

void FunctionAST::Print()
{
	std::cout << name << " ";

	for(int i = 0; i < args->OutputSize(); i++)
		PrintType((*args)[i]->InputType(0));

	std::cout << " -- ";

	for(int i = 0; i < body->OutputSize(); i++)
		PrintType((*body)[i]->OutputType(0));

	for(BodyAST::iterator it = body->begin(); it != body->end(); it++)
	{
		std::cout << std::endl << "  ";
		(*it)->Print();
	}

	std::cout << std::endl << std::endl;
}

const Type *FunctionAST::ConvertType(TypeAST t)
{
	switch(t)
	{
		case TYPE_NULL:
			throw std::string("not supported");

		case TYPE_INT32:
			return Type::Int32Ty;

		case TYPE_STRING:
			return PointerType::getUnqual(Type::Int8Ty);
			//return ArrayType::get(Type::Int8Ty, 5);
	}
}

void FunctionAST::Compile(Module *module)
{
	std::vector<const Type *> args(InputSize());
	for(size_t i = 0; i < args.size(); i++)
		args[i] = ConvertType(InputType(i));

	const Type *ret_type;
	switch(OutputSize())
	{
		case 0:
			ret_type = Type::VoidTy;
			break;
		case 1:
			ret_type = ConvertType(OutputType(0));
			break;
		default:
			std::vector<const Type *> ret_args(OutputSize());
			for(size_t i = 0; i < ret_args.size(); i++)
				ret_args[i] = ConvertType(OutputType(i));
			ret_type = StructType::get(ret_args);
			break;
	}

	FunctionType *function_type = FunctionType::get(ret_type, args, false);
	function = Function::Create(function_type, Function::ExternalLinkage, name, module);

	BasicBlock *entry = BasicBlock::Create("entry", function);
	IRBuilder<> builder(entry);

	// arg names
	Function::arg_iterator arg_it = function->arg_begin();
	for(int i = 0; i < InputSize(); i++)
	{
		std::ostringstream oss;
		oss << "inp" << i;
		arg_it->setName(oss.str());
		arg_it++;
	}

	assert(OutputSize() == body->size());
	Value *rets[OutputSize()];
	int ret_index = 0;
	for(BodyAST::iterator it = body->begin(); it != body->end(); it++)
	{
		AST *ast = *it;
		for(int i = 0; i < ast->OutputSize(); i++)
			rets[ret_index++] = ast->GetValue(i, builder);
	}
	assert(ret_index == OutputSize());

	switch(OutputSize())
	{
		case 0:
			builder.CreateRetVoid();
			break;
		case 1:
			builder.CreateRet(rets[0]);
			break;
		default:
			builder.CreateAggregateRet(rets, OutputSize());
			break;
	}
}

Function *FunctionAST::CompiledFunction()
{
	return function;
}

/// CallAST
CallAST::CallAST(FunctionAST *_function, BodyAST *_args)
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
	std::cout << function->Name(); 
}

void CallAST::Compile(IRBuilder<> builder)
{
	Function *compiled_function = function->CompiledFunction();
	std::vector<Value *> func_args;
	for(BodyAST::iterator it = args->begin(); it != args->end(); it++)
	{
		AST* arg = *it;
		for(int i = 0; i < arg->OutputSize(); i++)
		{
			Value *value = arg->GetValue(i, builder);
			func_args.push_back(value);
		}
	}
	Value *result = builder.CreateCall<std::vector<Value *>::iterator>(compiled_function, func_args.begin(), func_args.end());
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
ArgAST::ArgAST(int _n) : n(_n)
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
	return TYPE_INT32; // TODO inferir el tipo de argumento segun donde se use en el codigo
}

int ArgAST::InputSize()
{
	return 0;
}

int ArgAST::OutputSize()
{
	return 1;
}

void ArgAST::Print()
{
	std::cout << "arg" << n;
}

void ArgAST::Compile(IRBuilder<> builder)
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
		std::cout << "[";
		ast->Print();
		std::cout << "]:" << index;
	}
}

void OutputIndexAST::Compile(IRBuilder<> builder)
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
	arg1->Print();
	std::cout << " ";
	arg1->Print();
}

void DupAST::Compile(IRBuilder<> builder)
{
	SetValue(0, arg1->GetValue(0, builder));
	SetValue(1, arg1->GetValue(0, builder));
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
	std::cout << "(";
	arg1->Print();
	std::cout << "*";
	arg2->Print();
	std::cout << ")";
}

void MultAST::Compile(IRBuilder<> builder)
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
	std::cout << "(";
	arg1->Print();
	std::cout << "+";
	arg2->Print();
	std::cout << ")";
}

void AddAST::Compile(IRBuilder<> builder)
{
	SetValue(0, builder.CreateAdd(arg1->GetValue(0, builder), arg2->GetValue(0, builder)));
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
	std::cout << " ";
	arg1->Print();
}

void SwapAST::Compile(IRBuilder<> builder)
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
	std::cout << " ";
	arg2->Print();
	std::cout << " ";
	arg1->Print();
}

void OverAST::Compile(IRBuilder<> builder)
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
	std::cout << " ";
	arg3->Print();
	std::cout << " ";
	arg1->Print();
}

void RotAST::Compile(IRBuilder<> builder)
{
	SetValue(0, arg1->GetValue(0, builder));
	SetValue(2, arg2->GetValue(0, builder));
	SetValue(1, arg3->GetValue(0, builder));
}

