#include <iostream>
#include <list>
#include <sstream>
#include <assert.h>
#include <llvm/Module.h>
#include <llvm/Assembly/PrintModulePass.h>
#include <llvm/Function.h>
#include <llvm/CallingConv.h>
#include <llvm/Support/IRBuilder.h>
#include "parser.h"

using namespace llvm;

Parser::Parser(std::istream &in) : lexer(in)
{
}

FunctionBaseAST *Parser::FindFunction(const std::string &word)
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

AST *Parser::AppendCore(const std::string &word)
{
	if(word == "dup")
	{
		OutputIndexAST *arg1 = istack.Pop(TYPE_ANY);
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
	else if(word == "+.")
	{
		OutputIndexAST *arg2 = istack.Pop(TYPE_FLOAT);
		OutputIndexAST *arg1 = istack.Pop(TYPE_FLOAT);
		AST *ast = new AddFloatAST(arg1, arg2);
		istack.Push(ast);
		func_body->push_back(ast);
	}
	else if(word == "swap")
	{
		OutputIndexAST *arg2 = istack.Pop(TYPE_ANY);
		OutputIndexAST *arg1 = istack.Pop(TYPE_ANY);
		AST *ast = new SwapAST(arg1, arg2);
		istack.Push(ast);
		func_body->push_back(ast);
	}
	else if(word == "over")
	{
		OutputIndexAST *arg2 = istack.Pop(TYPE_ANY);
		OutputIndexAST *arg1 = istack.Pop(TYPE_ANY);
		AST *ast = new OverAST(arg1, arg2);
		istack.Push(ast);
		func_body->push_back(ast);
	}
	else if(word == "rot")
	{
		OutputIndexAST *arg3 = istack.Pop(TYPE_ANY);
		OutputIndexAST *arg2 = istack.Pop(TYPE_ANY);
		OutputIndexAST *arg1 = istack.Pop(TYPE_ANY);
		AST *ast = new RotAST(arg1, arg2, arg3);
		istack.Push(ast);
		func_body->push_back(ast);
	}
	else if(word == "\"")
	{
		lexer.ReadUntil(34);
		AST *ast = new StringAST(lexer.word);
		istack.Push(ast);
		func_body->push_back(ast);
		lexer.NextToken();
	}
	else if(word == "drop")
	{
		OutputIndexAST *arg1 = istack.Pop(TYPE_ANY);
		AST *ast = new DropAST(arg1);
		istack.Push(ast);
		func_body->push_back(ast);
	}
	else if(word == "i>s")
	{
		OutputIndexAST *arg1 = istack.Pop(TYPE_ANY);
		AST *ast = new CastIntToStringAST(arg1);
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

void Parser::ParseBody(const std::string &end)
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

FunctionAST *Parser::ParseFunction()
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

TypeAST Parser::ConvertType(const std::string &type)
{
	if(type == "i")
		return TYPE_INT32;
	else if(type == "s")
		return TYPE_STRING;
	else
		throw std::string("not supported");
}

ExternAST *Parser::ParseExtern()
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
		TypeAST token = ConvertType(lexer.word);
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
		TypeAST token = ConvertType(lexer.word);
		outputs.push_back(token);
		lexer.NextToken();
	}

	// parse )
	assert(lexer.word == ")");

	return new ExternAST(func_name, inputs, outputs);
}

void Parser::MainLoop()
{
	lexer.NextToken();
	while(true)
	{
		if(lexer.word == ":")
			functions.push_back(ParseFunction());
		else if(lexer.word == "extern")
			functions.push_back(ParseExtern());
		else
		{
			// TODO jit
			std::string error("jit not implemented: ");
			error += lexer.word;
			throw error;
		}

		lexer.NextToken();
	}
}

void Parser::Compile(Module *module)
{
	for(Functions::iterator it = functions.begin(); it != functions.end(); it++)
		(*it)->Compile(module);
}

void Parser::Print()
{
	for(Functions::iterator it = functions.begin(); it != functions.end(); it++)
		(*it)->Print();
}


