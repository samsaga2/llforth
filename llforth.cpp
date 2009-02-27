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
#include <llvm/CodeGen/Passes.h>
#include <llvm/LinkAllPasses.h>
#include <llvm/Target/TargetData.h>

using namespace llvm;

#include "lexer.h"
#include "ast.h"
#include "parser.h"

int main(int argc, char **argv)
{
	try
	{
		std::istringstream in(": test 1 2 over ; : test2 test * + ;");
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

