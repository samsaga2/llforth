#include <iostream>
#include <list>
#include <sstream>
#include <assert.h>
#include <llvm/Module.h>
#include <llvm/PassManager.h>
#include <llvm/Assembly/PrintModulePass.h>
#include <llvm/Function.h>
#include <llvm/CallingConv.h>
#include <llvm/Support/IRBuilder.h>
#include <llvm/CodeGen/Passes.h>
#include <llvm/LinkAllPasses.h>
#include <llvm/Target/TargetData.h>
#include "parser.h"

using namespace llvm;

Parser::Parser(Lexer &_lexer) : lexer(_lexer)
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
		AST *ast = new AddAST(arg1, arg2);
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
	else
	{
		std::string error("unknown token `");
		error += lexer.word;
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
		int integer = lexer.integer;
		lexer.NextToken();

		FunctionBaseAST *function = FindFunction(word);
		if(function != NULL)
		{
			// append function call
			BodyAST *args = new BodyAST();
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
					AST *ast = new IntegerAST(integer);
					istack.Push(ast);
					func_body->push_back(ast);
				}
				break;
			case Lexer::tok_eof:
				throw std::string("end of file");
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
	BodyAST *func_args = new BodyAST();
	for(BodyAST::iterator it = istack.args.begin(); it != istack.args.end(); it++)
		func_args->push_back(*it);

	// extract outputs
	BodyAST *func_outs = new BodyAST();
	for(std::list<InferenceStack::Counter *>::iterator it = istack.stack.begin(); it != istack.stack.end(); it++)
		func_outs->push_front((*it)->ast);

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
	while(lexer.word != "--" && lexer.token != Lexer::tok_eof)
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
	while(lexer.word != ")" && lexer.token != Lexer::tok_eof)
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
	while(true)
	{
		if(lexer.token == Lexer::tok_eof)
			break;
		else if(lexer.word == ":")
			functions.push_back(ParseFunction());
		else if(lexer.word == "extern")
			functions.push_back(ParseExtern());
		else
			// TODO jit
			throw std::string("not implemented");

		lexer.NextToken();
	}
}

void Parser::Compile(Module *module)
{
	for(Functions::iterator it = functions.begin(); it != functions.end(); it++)
		(*it)->Compile(module);
}

void Parser::Optimize(Module *module)
{
	PassManager pm;
	pm.add(new TargetData(module));
	pm.add(createIPSCCPPass());
	pm.add(createGlobalOptimizerPass());
	pm.add(createInstructionCombiningPass());
	pm.add(createFunctionInliningPass());
	pm.add(createGlobalOptimizerPass());
	pm.add(createInstructionCombiningPass());
	pm.add(createGVNPass());
	pm.add(createPromoteMemoryToRegisterPass());
	pm.add(createCFGSimplificationPass());
	pm.run(*module);
}

void Parser::Print()
{
	for(Functions::iterator it = functions.begin(); it != functions.end(); it++)
		(*it)->Print();
}


