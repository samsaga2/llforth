#include <iostream>
#include <fstream>
#include "lexer.h"
#include "ast.h"
#include "parser.h"

int main(int argc, char **argv)
{
	try
	{
		std::ifstream in("test.llfs");
		Lexer lexer(in);
		lexer.NextToken();

		Parser parser(lexer);
		parser.MainLoop();
		parser.Compile();
	}
	catch(std::string &error)
	{
		std::cout << "Exception: " << error << std::endl;
	}
}

