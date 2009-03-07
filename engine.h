#pragma once

#include <llvm/Module.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include "istack.h"
#include "lexer.h"
#include "ast.h"

class UnknownToken : std::exception
{
	virtual const char *what() const throw()
	{
		return "Unknown token";
	}
};

class Engine;

class Word
{
public:
	virtual const char *Name() = 0;
	virtual void Execute(Engine *engine, Lexer *lexer, int state) = 0;
};

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

	std::list<Word *> words;
public:
	Engine(std::istream &in, llvm::Module *module);
	~Engine();
	FunctionBaseAST *FindFunction(const std::string &word);
	void Compile();
	void Print();
	void Execute(const std::string &word);
	void Execute(FunctionBaseAST *function);
	void PrintJITStack();
	InferenceStack *IStack() { return &istack; }
	void AppendBody(AST *ast);
	void ParseFunction();
	void ParseExtern();
private:
	void AppendCore(const std::string &word, int state);
	void ParseBody(const std::string &end);
};

