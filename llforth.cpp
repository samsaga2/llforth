#include <iostream>
#include <fstream>
#include "lexer.h"
#include "ast.h"
#include "parser.h"
#include <llvm/Module.h>
#include <llvm/Bitcode/ReaderWriter.h>

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

		std::ofstream of("a.obj", std::ios::binary);
		WriteBitcodeToFile(&module, of);
		of.close();
	}
	catch(std::string &error)
	{
		std::cout << "Exception: " << error << std::endl;
	}
}

