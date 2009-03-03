#pragma once

#include "istack.h"
#include "lexer.h"
#include "ast.h"

class Parser
{
	Lexer lexer;
	typedef std::list<FunctionBaseAST *> Functions;
	Functions functions;
	InferenceStack istack;
	BodyAST *func_body;
public:
	Parser(std::istream &in);
	FunctionBaseAST *FindFunction(const std::string &word);
	void MainLoop();
	void Compile(Module *module);
	void Optimize(Module *module);
	void Print();
private:
	AST *AppendCore(const std::string &word);
	void ParseBody(const std::string &end);
	FunctionAST *ParseFunction();
	ExternAST *ParseExtern();
	TypeAST ConvertType(const std::string &type);
};

