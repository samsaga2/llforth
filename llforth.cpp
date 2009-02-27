#include <iostream>
#include <sstream>
#include "lexer.h"
#include "ast.h"
#include "parser.h"

int main(int argc, char **argv)
{
	try
	{
		std::istringstream in(": test 1 2 ; : test2 dup ; : test3 test test2 over + + ;");
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

