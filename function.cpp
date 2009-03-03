#include "function.h"
#include <iostream>
#include <sstream>

using namespace std;

/// FunctionBaseAST
const Type *FunctionBaseAST::ConvertType(TypeAST t)
{
	switch(t)
	{
		case TYPE_NULL:
			throw string("not supported");

		case TYPE_INT32:
			return Type::Int32Ty;

		case TYPE_STRING:
			return PointerType::getUnqual(Type::Int8Ty);
			//return ArrayType::get(Type::Int8Ty, 5);
	}
}

/// FunctionAST
FunctionAST::FunctionAST(const string &_name, BodyAST *_body, list<ArgAST *> *_args, list<OutputIndexAST *> *_outputs)
	: name(_name), body(_body), args(_args), outputs(_outputs)
{
}

const string FunctionAST::Name()
{
	return name;
}

TypeAST FunctionAST::InputType(int index)
{
	ArgumentsList::iterator it = args->begin();
	while(it != args->end() && index > 0)
	{
		index--;
		it++;
	}
	return (*it)->OutputType(0);
}

TypeAST FunctionAST::OutputType(int index)
{
	OutputList::iterator it = outputs->begin();
	while(it != outputs->end() && index > 0)
	{
		index--;
		it++;
	}
	return (*it)->OutputType();
}

int FunctionAST::InputSize()
{
	return args->size();
}

int FunctionAST::OutputSize()
{
	return outputs->size();
}

void PrintType(TypeAST t)
{
	switch(t)
	{
		case TYPE_NULL: cout << " *"; break;
		case TYPE_INT32: cout << " i"; break;
		case TYPE_STRING: cout << " s"; break;
		case TYPE_ANY: cout << " ?"; break;
	}
}

void FunctionAST::Print()
{
	cout << ": " << name << " (";

	for(size_t i = 0; i < args->size(); i++)
		PrintType(InputType(i));

	cout << " --";

	for(size_t i = 0; i < outputs->size(); i++)
		PrintType(OutputType(i));

	cout << " )";

	for(BodyAST::iterator it = body->begin(); it != body->end(); it++)
	{
		cout << " ";
		(*it)->Print();
	}
	cout << " ;" << endl;
}

void FunctionAST::Compile(Module *module)
{
	// function arguments types
	vector<const Type *> args(InputSize());
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
			vector<const Type *> ret_args(OutputSize());
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
		ostringstream oss;
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
	vector<Value *> rets;
	for(OutputList::iterator it = outputs->begin(); it != outputs->end(); it++)
		rets.push_back((*it)->GetValue(builder));

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
ExternAST::ExternAST(const string &_name, vector<TypeAST> _inputs, vector<TypeAST> _outputs)
	: name(_name), inputs(_inputs), outputs(_outputs)
{
}

const string ExternAST::Name()
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
	cout << "extern " << name << " (";

	for(int i = 0; i < inputs.size(); i++)
		PrintType(inputs[i]);

	cout << " --";

	for(int i = 0; i < outputs.size(); i++)
		PrintType(outputs[i]);

	cout << " )" << endl;
}

void ExternAST::Compile(Module *module)
{
	// function arguments types
	vector<const Type *> args(inputs.size());
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
			vector<const Type *> ret_args(outputs.size());
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
		ostringstream oss;
		oss << "inp" << i;
		arg_it->setName(oss.str());
		arg_it++;
	}
}

Function *ExternAST::CompiledFunction()
{
	return function;
}

