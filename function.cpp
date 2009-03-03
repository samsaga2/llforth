#include "function.h"
#include <iostream>
#include <sstream>

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

/// FunctionAST
FunctionAST::FunctionAST(const std::string &_name, BodyAST *_body, BodyAST *_args, BodyAST *_outputs)
	: name(_name), body(_body), args(_args), outputs(_outputs)
{
}

const std::string FunctionAST::Name()
{
	return name;
}

TypeAST FunctionAST::InputType(int index)
{
	return (*args)[index]->OutputType(0);
}

TypeAST FunctionAST::OutputType(int index)
{
	return outputs->OutputType(index);
}

int FunctionAST::InputSize()
{
	return args->OutputSize();
}

int FunctionAST::OutputSize()
{
	return outputs->OutputSize();
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

void FunctionAST::Print()
{
	std::cout << ": " << name << " (";

	for(int i = 0; i < args->OutputSize(); i++)
		PrintType((*args)[i]->OutputType(0));

	std::cout << " --";

	for(int i = 0; i < outputs->OutputSize(); i++)
	{
		AST* ast = (*outputs)[i];
		for(size_t j = 0; j < ast->OutputSize(); j++)
			PrintType(ast->OutputType(j));
	}
	std::cout << " )";

	for(BodyAST::iterator it = body->begin(); it != body->end(); it++)
	{
		std::cout << " ";
		(*it)->Print();
	}
	std::cout << " ;" << std::endl << std::endl;
}

void FunctionAST::Compile(Module *module)
{
	// function arguments types
	std::vector<const Type *> args(InputSize());
	for(size_t i = 0; i < args.size(); i++)
		args[i] = ConvertType(InputType(i));

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

	Function::arg_iterator arg_it = function->arg_begin();
	for(int i = 0; i < InputSize(); i++)
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
	for(BodyAST::iterator it = body->begin(); it != body->end(); it++)
		(*it)->Compile(builder);

	// create outputs
	std::vector<Value *> rets;
	for(BodyAST::iterator it = outputs->begin(); it != outputs->end(); it++)
	{
		AST *ast = *it;
		for(int i = 0; i < ast->OutputSize(); i++)
			rets.push_back(ast->GetValue(i, builder));
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

/// ExternAST
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

void ExternAST::Compile(Module *module)
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

