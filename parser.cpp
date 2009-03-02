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

AST *Parser::AppendCore(const std::string &word)
{
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
	else if(word == "drop")
	{
		OutputIndexAST *arg1 = istack.Pop();
		istack.Push(new DropAST(arg1));
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

		FunctionAST *function = FindFunction(word);
		if(function != NULL)
		{
			// append function call
			BodyAST *args = istack.Pop(function->InputSize());
			istack.Push(new CallAST(function, args));
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
				istack.Push(new IntegerAST(integer));
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
	ParseBody(";");

	// extract function body
	BodyAST *func_body = new BodyAST();
	for(std::list<InferenceStack::Counter *>::iterator it = istack.stack.begin(); it != istack.stack.end(); it++)
		func_body->push_front((*it)->ast);

	// extract function args
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
		else if(lexer.word == ":")
		{
			functions.push_back(ParseFunction());
			functions.back()->Print();
		}
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

