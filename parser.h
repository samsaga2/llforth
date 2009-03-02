#pragma once

#include "istack.h"
#include "lexer.h"
#include "ast.h"

class Parser
{
public:
	Lexer lexer;
	typedef std::list<FunctionAST *> Functions;
	Functions functions;
	InferenceStack istack;

	Parser(Lexer &_lexer);
	FunctionAST *FindFunction(const std::string &word);
	void MainLoop();
	void Compile(Module *module);
private:
	AST *AppendCore(const std::string &word);
	void ParseBody(const std::string &end);
	FunctionAST *ParseFunction();
};

