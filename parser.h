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
	void AppendFunction();
	AST *AppendCore();
	void AppendInteger();
	void ParseWordExpr();
	void ParseBody(const std::string &end);
	FunctionAST *ParseFunction();
	void MainLoop();
	void Compile();
};

