#pragma once

#include <llvm/Module.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include "istack.h"
#include "lexer.h"
#include "ast.h"

class Engine
{
	Lexer lexer;
	typedef std::list<FunctionBaseAST *> Functions;
	Functions functions;
	InferenceStack istack;
	BodyAST *func_body;
	llvm::Module *module;
	llvm::ExecutionEngine *jit;

	typedef std::pair<TypeAST, GenericValue> jit_value;
	std::list<jit_value> jit_stack;
public:
	Engine(std::istream &in, llvm::Module *module);
	FunctionBaseAST *FindFunction(const std::string &word);
	void Compile();
	void Print();
	void Execute(const std::string &word);
	void Execute(FunctionBaseAST *function);
	void PrintJITStack();
private:
	AST *AppendCore(const std::string &word);
	void ParseBody(const std::string &end);
	FunctionAST *ParseFunction();
	ExternAST *ParseExtern();
};

