#include <iostream>
#include <sstream>
#include "lexer.h"
#include "ast.h"
#include "parser.h"

int main(int argc, char **argv)
{
	try
	{
		std::istringstream in(": test 3 1 + \" hola\" swap ;");
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

