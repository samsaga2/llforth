#include <iostream>
#include <list>
#include <sstream>
#include <assert.h>
#include <llvm/Module.h>
#include <llvm/PassManager.h>
#include <llvm/Assembly/PrintModulePass.h>
#include <llvm/Function.h>
#include <llvm/CallingConv.h>
#include <llvm/Support/IRBuilder.h>

using namespace llvm;

#include "lexer.h"
#include "ast.h"
#include "parser.h"

int main(int argc, char **argv)
{
	try
	{
		std::istringstream in(": test dup * 5 + ; : test2 dup + + 5 ;");
		Lexer lexer(in);
		lexer.NextToken();

		Parser parser(lexer);
		parser.MainLoop();
	}
	catch(std::string &error)
	{
		std::cout << "Exception: " << error << std::endl;
	}
}

