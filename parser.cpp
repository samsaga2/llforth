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

using namespace llvm;

#include "parser.h"

Parser::Parser(Lexer &_lexer) : lexer(_lexer)
{
}

FunctionAST *Parser::FindFunction(const std::string &word)
{
	Functions::reverse_iterator it = functions.rbegin();
	while(it != functions.rend())
	{
		FunctionAST *function = *it;
		if(function->Name() == word)
			return function;
		it++;
	}
	return NULL;
}

void Parser::AppendFunction()
{
	std::string func_name = lexer.word;
	lexer.NextToken();

	FunctionAST *function = FindFunction(func_name);
	assert(function != NULL);

	BodyAST *args = istack.Pop(function->InputSize());
	CallAST *call = new CallAST(function, args);
	istack.Push(call);
}

AST *Parser::AppendCore()
{
	std::string word = lexer.word;
	lexer.NextToken();

	if(word == "dup")
	{
		OutputIndexAST *arg1 = istack.Pop();
		istack.Push(new DupAST(arg1));
	}
	else if(word == "*")
	{
		OutputIndexAST *arg2 = istack.Pop();
		OutputIndexAST *arg1 = istack.Pop();
		istack.Push(new MultAST(arg1, arg2));
	}
	else if(word == "+")
	{
		OutputIndexAST *arg2 = istack.Pop();
		OutputIndexAST *arg1 = istack.Pop();
		istack.Push(new AddAST(arg1, arg2));
	}
	else if(word == "swap")
	{
		OutputIndexAST *arg2 = istack.Pop();
		OutputIndexAST *arg1 = istack.Pop();
		istack.Push(new SwapAST(arg1, arg2));
	}
	else if(word == "over")
	{
		OutputIndexAST *arg2 = istack.Pop();
		OutputIndexAST *arg1 = istack.Pop();
		istack.Push(new OverAST(arg1, arg2));
	}
	else if(word == "rot")
	{
		OutputIndexAST *arg3 = istack.Pop();
		OutputIndexAST *arg2 = istack.Pop();
		OutputIndexAST *arg1 = istack.Pop();
		istack.Push(new RotAST(arg1, arg2, arg3));
	}
	else if(word == "\"")
	{
		lexer.ReadUntil(34);
		istack.Push(new StringAST(lexer.word));
		lexer.NextToken();
	}
	else if(word == "--")
	{
		lexer.ReadLine();
		lexer.NextToken();
	}
	else
	{
		std::string error("unknown token `");
		error += lexer.word;
		error += "'";
		throw error;
	}
}

void Parser::AppendInteger()
{
	int integer = lexer.integer;
	lexer.NextToken();

	AST *ast = new IntegerAST(integer);
	istack.Push(ast);
}

void Parser::ParseWordExpr()
{
	FunctionAST *function = FindFunction(lexer.word);
	if(function != NULL)
		AppendFunction();
	else
		switch(lexer.token)
		{
		default:
			throw std::string("end of file");
		case Lexer::tok_word:
			AppendCore();
			break;
		case Lexer::tok_integer:
			AppendInteger();
			break;
		}
}

void Parser::ParseBody(const std::string &end)
{
	istack.Clear();
	do
	{
		if(lexer.word == end)
			break;

		ParseWordExpr();
	}
	while(true);
}

FunctionAST *Parser::ParseFunction()
{
	assert(lexer.word == ":");
	lexer.NextToken();

	std::string func_name = lexer.word;
	lexer.NextToken();

	ParseBody(";");

	BodyAST *func_body = new BodyAST();
	while(istack.stack.size())
		func_body->push_front(istack.Pop());

	BodyAST *func_args = new BodyAST();
	for(BodyAST::iterator it = istack.args.begin(); it != istack.args.end(); it++)
		func_args->push_back(*it);

	return new FunctionAST(func_name, func_body, func_args);
}

void Parser::MainLoop()
{
	while(true)
	{
		if(lexer.token == Lexer::tok_eof)
			break;
		else if(lexer.word == "--")
		{
			lexer.ReadLine();
			lexer.NextToken();
		}
		else
		{
			FunctionAST *function = ParseFunction();
			functions.push_back(function);
			function->Print();
			lexer.NextToken();
		}
	}
}

void Parser::Compile()
{
	Module module("llforth");

	for(Functions::iterator it = functions.begin(); it != functions.end(); it++)
		(*it)->Compile(&module);

	PassManager pm;
	pm.add(new TargetData(&module));
	pm.add(createIPSCCPPass());
	pm.add(createGlobalOptimizerPass());
	pm.add(createInstructionCombiningPass());
	pm.add(createFunctionInliningPass());
	pm.add(createGlobalOptimizerPass());
	pm.add(createInstructionCombiningPass());
	pm.add(createGVNPass());
	pm.add(createPromoteMemoryToRegisterPass());
	pm.add(createCFGSimplificationPass());
	pm.run(module);

	module.dump();
}

