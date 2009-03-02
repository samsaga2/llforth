#include <iostream>
#include <fstream>
#include "lexer.h"
#include "ast.h"
#include "parser.h"
#include <llvm/Module.h>

using namespace llvm;

int main(int argc, char **argv)
{
	try
	{
		std::ifstream in("test.llfs");
		Lexer lexer(in);
		lexer.NextToken();

		Parser parser(lexer);
		parser.MainLoop();

		Module module("llforth");
		parser.Compile(&module);
		module.dump();
	}
	catch(std::string &error)
	{
		std::cout << "Exception: " << error << std::endl;
	}
}

