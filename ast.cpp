#include "ast.h"
#include <iostream>
#include <sstream>

AST::AST() : compiled(false)
{
}

void AST::Compile(IRBuilder<> builder)
{
	if(!compiled)
	{
		DoCompile(builder);
		compiled = true;
	}
	else
		assert(false);	
}

Value *AST::GetValue(int index)
{
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

void IntegerAST::DoCompile(IRBuilder<> builder)
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

void StringAST::DoCompile(IRBuilder<> builder)
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
	return body->OutputType(index);
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
		case TYPE_ANY: std::cout << " ?"; break;
	}
}

/// FunctionBaseAST
const Type *FunctionBaseAST::ConvertType(TypeAST t)
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

// FunctionAST
void FunctionAST::Print()
{
	std::cout << name << " (";

	for(int i = 0; i < args->OutputSize(); i++)
		PrintType((*args)[i]->OutputType(0));

	std::cout << " --";

	for(int i = 0; i < body->OutputSize(); i++)
	{
		AST* ast = (*body)[i];
		for(size_t j = 0; j < ast->OutputSize(); j++)
			PrintType(ast->OutputType(j));
	}
	std::cout << " )";

	for(BodyAST::iterator it = body->begin(); it != body->end(); it++)
	{
		std::cout << std::endl << "  ";
		(*it)->Print();
	}

	std::cout << std::endl << std::endl;
}

void FunctionAST::DoCompile(Module *module)
{
	// function arguments types
	std::vector<const Type *> args(OutputSize());
	for(size_t i = 0; i < args.size(); i++)
		args[i] = ConvertType(OutputType(i));

	// function output types
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

	// create function
	FunctionType *function_type = FunctionType::get(ret_type, args, false);
	function = Function::Create(function_type, Function::ExternalLinkage, name, module);

	// arg names
	Function::arg_iterator arg_it = function->arg_begin();
	for(int i = 0; i < OutputSize(); i++)
	{
		std::ostringstream oss;
		oss << "inp" << i;
		arg_it->setName(oss.str());
		arg_it++;
	}

	// function main entry
	BasicBlock *entry = BasicBlock::Create("entry", function);
	IRBuilder<> builder(entry);

	// create body
	std::vector<Value *> rets;
	for(BodyAST::iterator it = body->begin(); it != body->end(); it++)
	{
		AST *ast = *it;
		for(int i = 0; i < ast->OutputSize(); i++)
		{
			ast->Compile(builder);
			rets.push_back(ast->GetValue(i));
		}
	}

	// create outputs
	switch(OutputSize())
	{
		case 0:
			builder.CreateRetVoid();
			break;
		case 1:
			builder.CreateRet(rets[0]);
			break;
		default:
			builder.CreateAggregateRet(&rets[0], OutputSize());
			break;
	}
}

Function *FunctionAST::CompiledFunction()
{
	return function;
}

// ExternAST
ExternAST::ExternAST(const std::string &_name, std::vector<TypeAST> _inputs, std::vector<TypeAST> _outputs)
	: name(_name), inputs(_inputs), outputs(_outputs)
{
}

const std::string ExternAST::Name()
{
	return name;
}

TypeAST ExternAST::InputType(int index)
{
	assert(index >= 0 && index < inputs.size());
	return inputs[index];
}

TypeAST ExternAST::OutputType(int index)
{
	assert(index >= 0 && index < outputs.size());
	return outputs[index];
}

int ExternAST::InputSize()
{
	return inputs.size();
}

int ExternAST::OutputSize()
{
	return outputs.size();
}

void ExternAST::Print()
{
	std::cout << "extern " << name << " (";

	for(int i = 0; i < inputs.size(); i++)
		PrintType(inputs[i]);

	std::cout << " --";

	for(int i = 0; i < outputs.size(); i++)
		PrintType(outputs[i]);

	std::cout << " )";

	std::cout << std::endl << std::endl;
}

void ExternAST::DoCompile(Module *module)
{
	// function arguments types
	std::vector<const Type *> args(inputs.size());
	for(size_t i = 0; i < inputs.size(); i++)
		args[i] = ConvertType(inputs[i]);

	// function output types
	const Type *ret_type;
	switch(outputs.size())
	{
		case 0:
			ret_type = Type::VoidTy;
			break;
		case 1:
			ret_type = ConvertType(outputs[0]);
			break;
		default:
			std::vector<const Type *> ret_args(outputs.size());
			for(size_t i = 0; i < ret_args.size(); i++)
				ret_args[i] = ConvertType(outputs[i]);
			ret_type = StructType::get(ret_args);
			break;
	}

	// create function
	FunctionType *function_type = FunctionType::get(ret_type, args, false);
	function = Function::Create(function_type, Function::ExternalLinkage, name, module);

	// arg names
	Function::arg_iterator arg_it = function->arg_begin();
	for(int i = 0; i < inputs.size(); i++)
	{
		std::ostringstream oss;
		oss << "inp" << i;
		arg_it->setName(oss.str());
		arg_it++;
	}
}

Function *ExternAST::CompiledFunction()
{
	return function;
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
	std::cout << function->Name(); 
}

void CallAST::DoCompile(IRBuilder<> builder)
{
	Function *compiled_function = function->CompiledFunction();
	std::vector<Value *> func_args;
	for(BodyAST::iterator it = args->begin(); it != args->end(); it++)
	{
		AST* arg = *it;
		for(int i = 0; i < arg->OutputSize(); i++)
		{
			arg->Compile(builder);
			Value *value = arg->GetValue(i);
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
	return TYPE_ANY;
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
	std::cout << "arg" << n;
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
		std::cout << "[";
		ast->Print();
		std::cout << "]:" << index;
	}
}

void OutputIndexAST::DoCompile(IRBuilder<> builder)
{
	ast->Compile(builder);
	SetValue(0, ast->GetValue(index));
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

void DupAST::DoCompile(IRBuilder<> builder)
{
	arg1->Compile(builder);
	SetValue(0, arg1->GetValue(0));
	SetValue(1, arg1->GetValue(0));
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

void MultAST::DoCompile(IRBuilder<> builder)
{
	arg1->Compile(builder),
	arg2->Compile(builder);
	SetValue(0, builder.CreateMul(arg1->GetValue(0), arg2->GetValue(0)));
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

void AddAST::DoCompile(IRBuilder<> builder)
{
	arg1->Compile(builder);
	arg2->Compile(builder);
	SetValue(0, builder.CreateAdd(arg1->GetValue(0), arg2->GetValue(0)));
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

void SwapAST::DoCompile(IRBuilder<> builder)
{
	arg1->Compile(builder),
	arg2->Compile(builder);
	SetValue(0, arg1->GetValue(0));
	SetValue(1, arg2->GetValue(0));
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

void OverAST::DoCompile(IRBuilder<> builder)
{
	arg1->Compile(builder);
	arg2->Compile(builder);
	SetValue(0, arg1->GetValue(0));
	SetValue(1, arg2->GetValue(0));
	SetValue(2, arg1->GetValue(0));
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

void RotAST::DoCompile(IRBuilder<> builder)
{
	arg1->Compile(builder);
	arg2->Compile(builder);
	SetValue(0, arg1->GetValue(0));
	SetValue(2, arg2->GetValue(0));
	SetValue(1, arg3->GetValue(0));
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
	std::cout << "[ ";
	arg1->Print();
	std::cout << " ]:*";
}

void DropAST::DoCompile(IRBuilder<> builder)
{
	arg1->Compile(builder);
}

