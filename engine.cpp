#include <iostream>
#include <list>
#include <sstream>
#include <assert.h>
#include <llvm/Assembly/PrintModulePass.h>
#include <llvm/Function.h>
#include <llvm/CallingConv.h>
#include <llvm/Support/IRBuilder.h>
#include "llvm/Support/raw_ostream.h"
#include "engine.h"

using namespace llvm;

Engine::Engine(std::istream &in, Module *_module) : lexer(in)
{
	module = _module;
	jit = ExecutionEngine::create(module);
}

FunctionBaseAST *Engine::FindFunction(const std::string &word)
{
	Functions::reverse_iterator it = functions.rbegin();
	while(it != functions.rend())
	{
		FunctionBaseAST *function = *it;
		if(function->Name() == word)
			return function;
		it++;
	}
	return NULL;
}

AST *Engine::AppendCore(const std::string &word)
{
	if(word == "dup")
	{
		OutputIndexAST *arg1 = istack.Pop(TYPE_INT32);
		AST *ast = new DupAST(arg1);
		istack.Push(ast);
		func_body->push_back(ast);
	}
	else if(word == "*")
	{
		OutputIndexAST *arg2 = istack.Pop(TYPE_INT32);
		OutputIndexAST *arg1 = istack.Pop(TYPE_INT32);
		AST *ast = new MultAST(arg1, arg2);
		istack.Push(ast);
		func_body->push_back(ast);
	}
	else if(word == "+")
	{
		OutputIndexAST *arg2 = istack.Pop(TYPE_INT32);
		OutputIndexAST *arg1 = istack.Pop(TYPE_INT32);
		AST *ast = new AddIntegerAST(arg1, arg2);
		istack.Push(ast);
		func_body->push_back(ast);
	}
	else if(word == "f+")
	{
		OutputIndexAST *arg2 = istack.Pop(TYPE_FLOAT);
		OutputIndexAST *arg1 = istack.Pop(TYPE_FLOAT);
		AST *ast = new AddFloatAST(arg1, arg2);
		istack.Push(ast);
		func_body->push_back(ast);
	}
	else if(word == "swap")
	{
		OutputIndexAST *arg2 = istack.Pop(TYPE_INT32);
		OutputIndexAST *arg1 = istack.Pop(TYPE_INT32);
		AST *ast = new SwapAST(arg1, arg2);
		istack.Push(ast);
		func_body->push_back(ast);
	}
	else if(word == "over")
	{
		OutputIndexAST *arg2 = istack.Pop(TYPE_INT32);
		OutputIndexAST *arg1 = istack.Pop(TYPE_INT32);
		AST *ast = new OverAST(arg1, arg2);
		istack.Push(ast);
		func_body->push_back(ast);
	}
	else if(word == "rot")
	{
		OutputIndexAST *arg3 = istack.Pop(TYPE_INT32);
		OutputIndexAST *arg2 = istack.Pop(TYPE_INT32);
		OutputIndexAST *arg1 = istack.Pop(TYPE_INT32);
		AST *ast = new RotAST(arg1, arg2, arg3);
		istack.Push(ast);
		func_body->push_back(ast);
	}
	else if(word == "s\"")
	{
		lexer.ReadUntil(34);
		AST *ast = new StringAST(lexer.word);
		istack.Push(ast);
		func_body->push_back(ast);
		lexer.NextToken();
	}
	else if(word == "drop")
	{
		OutputIndexAST *arg1 = istack.Pop(TYPE_INT32);
		AST *ast = new DropAST(arg1);
		istack.Push(ast);
		func_body->push_back(ast);
	}
	else
	{
		std::string error("unknown token `");
		error += word;
		error += "'";
		throw error;
	}
}

void Engine::ParseBody(const std::string &end)
{
	istack.Clear();
	do
	{
		if(lexer.word == end)
			break;

		std::string word = lexer.word;
		int token = lexer.token;
		int number_integer = lexer.number_integer;
		int number_float = lexer.number_float;
		lexer.NextToken();

		FunctionBaseAST *function = FindFunction(word);
		if(function != NULL)
		{
			// append function call
			OutputList *args = new OutputList();
			for(size_t i = 0; i < function->InputSize(); i++)
				args->push_back(istack.Pop(function->InputType(i)));

			AST *ast = new CallAST(function, args);
			istack.Push(ast);
			func_body->push_back(ast);
		}
		else
			switch(token)
			{
			case Lexer::tok_word:
				// append core word
				AppendCore(word);
				break;
			case Lexer::tok_integer:
				// append literal integer
				{
					AST *ast = new IntegerAST(number_integer);
					istack.Push(ast);
					func_body->push_back(ast);
				}
				break;
			case Lexer::tok_float:
				// append literal float
				{
					AST *ast = new FloatAST(number_float);
					istack.Push(ast);
					func_body->push_back(ast);
				}
				break;
			}
	}
	while(true);
}

FunctionAST *Engine::ParseFunction()
{
	// parse :
	assert(lexer.word == ":");
	lexer.NextToken();

	// parse function name
	std::string func_name = lexer.word;
	lexer.NextToken();

	// parse function body
	func_body = new BodyAST();
	ParseBody(";");

	// extract function args
	ArgumentsList *func_args = new ArgumentsList();
	for(ArgumentsList::iterator it = istack.args.begin(); it != istack.args.end(); it++)
		func_args->push_back(*it);

	// extract outputs
	OutputList *func_outs = new OutputList();
	for(OutputList::iterator it = istack.stack.begin(); it != istack.stack.end(); it++)
		func_outs->push_back(*it);

	return new FunctionAST(func_name, func_body, func_args, func_outs);
}

ExternAST *Engine::ParseExtern()
{
	// parse extern
	assert(lexer.word == "extern");
	lexer.NextToken();

	// parse function name
	std::string func_name = lexer.word;
	lexer.NextWord();

	// parse (
	assert(lexer.word == "(");
	lexer.NextToken();

	// read inputs
	std::vector<TypeAST> inputs;
	while(lexer.word != "--")
	{
		TypeAST token = ConvertTypeAST(lexer.word);
		inputs.push_back(token);
		lexer.NextToken();
	}

	// parse --
	assert(lexer.word == "--");
	lexer.NextToken();

	// read outputs
	std::vector<TypeAST> outputs;
	while(lexer.word != ")")
	{
		TypeAST token = ConvertTypeAST(lexer.word);
		outputs.push_back(token);
		lexer.NextToken();
	}

	// parse )
	assert(lexer.word == ")");

	return new ExternAST(func_name, inputs, outputs);
}

void Engine::Execute(FunctionBaseAST *function)
{
	std::vector<GenericValue> args;
	for(size_t i = 0; i < function->InputSize(); i++)
	{
		assert(!jit_stack.empty());
		GenericValue v = jit_stack.back().second;
		args.push_back(v);
		jit_stack.pop_back();
	}

	Function *f = function->CompiledFunction();
	GenericValue result = jit->runFunction(f, args);

	switch(function->OutputSize())
	{
	case 0: break;
	case 1: jit_stack.push_back(jit_value(function->OutputType(0), result)); break;
	case 2: throw std::string("not supported multiple return values");
	}
}

void Engine::Execute(const std::string &word)
{
	GenericValue gv;
	FunctionBaseAST *function = FindFunction(word);
	if(function != NULL)
		Execute(function);
	else if(lexer.word == ".s")
		PrintJITStack();
	else
		switch(lexer.token)
		{
		case Lexer::tok_integer:
			gv.IntVal = APInt(32, lexer.number_integer);
			jit_stack.push_back(jit_value(TYPE_INT32, gv));
			break;

		case Lexer::tok_float:
			gv.FloatVal = lexer.number_float;
			jit_stack.push_back(jit_value(TYPE_FLOAT, gv));
			break;

		default:
			std::string error("jit not implemented: ");
			error += lexer.word;
			throw error;
		}
}

void Engine::Compile()
{
	// construct ast
	try
	{
		lexer.NextToken();
		while(true)
		{
			if(lexer.word == ":")
			{
				FunctionBaseAST *f = ParseFunction();
				f->Compile(module);
				functions.push_back(f);
			}
			else if(lexer.word == "extern")
			{
				FunctionBaseAST *f = ParseExtern();
				f->Compile(module);
				functions.push_back(f);
			}
			else
				Execute(lexer.word);

			lexer.NextToken();
		}
	}
	catch(EndOfStream &e)
	{
	}
}

void Engine::Print()
{
	for(Functions::iterator it = functions.begin(); it != functions.end(); it++)
		(*it)->Print();
}

void Engine::PrintJITStack()
{
	for(std::list<jit_value>::iterator it = jit_stack.begin();
	    it != jit_stack.end();
	    it++)
	{
		switch(it->first)
		{
		case TYPE_INT32:
			std::cout << "  " << it->second.IntVal.toString(10,true);
			break;
		case TYPE_FLOAT:
			std::cout << "  " << it->second.FloatVal;
			break;
		}
		std::cout << std::endl;
	}
}

