#include <iostream>
#include <fstream>
#include <sstream>
#include <llvm/Module.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <unistd.h>
#include <llvm/PassManager.h>
#include <llvm/CodeGen/Passes.h>
#include <llvm/LinkAllPasses.h>
#include <llvm/Target/TargetData.h>
#include "lexer.h"
#include "ast.h"
#include "engine.h"

using namespace llvm;

static bool verbose = false;
static std::string input_filename("");
static std::string output_filename("a.obj");
static bool optimize = false;

void show_help()
{
	std::cout << "llforth [OPTIONS]..." << std::endl << std::endl;
	std::cout << "  -h         	show help" << std::endl;
	std::cout << "  -v         	verbose output" << std::endl;
	std::cout << "  -o filename	object filename" << std::endl;
	std::cout << "  -O         	run optimize passes" << std::endl;
	std::cout << "  -i         	input filename" << std::endl;
	exit(0);
}

void read_args(int argc, char **argv)
{
	int c;
	extern char *optarg;
	extern int optopt;

	while((c = getopt(argc, argv, "vho:Oi:")) != -1)
		switch(c)
		{
		case 'h':
			show_help();
			break;
		case 'v':
			verbose = true;
			break;
		case 'o':
			output_filename = optarg;
			break;
		case 'O':
			optimize = true;
			break;
		case 'i':
			input_filename = optarg;
			break;
		case '?':
			cerr << "Unknown option -" << (char)optopt << std::endl;
		}
}

void do_optimize(Module *module)
{
	PassManager pm;
	pm.add(new TargetData(module));
	pm.add(createIPSCCPPass());
	pm.add(createGlobalOptimizerPass());
	pm.add(createInstructionCombiningPass());
	pm.add(createFunctionInliningPass());
	pm.add(createGlobalOptimizerPass());
	pm.add(createInstructionCombiningPass());
	pm.add(createGVNPass());
	pm.add(createPromoteMemoryToRegisterPass());
	pm.add(createCFGSimplificationPass());
	pm.run(*module);
}

void compile(std::istream &in)
{
	Module module("llforth");

	Engine engine(in, &module);
	engine.Compile();

	if(optimize) do_optimize(&module);
	if(verbose)  module.dump();

	// save object file
	std::ofstream of(output_filename.c_str(), std::ios::binary);
	WriteBitcodeToFile(&module, of);
	of.close();
}

int main(int argc, char **argv)
{
	read_args(argc, argv);
	try
	{
		if(input_filename.size() != 0)
		{
			std::ifstream is(input_filename.c_str());
			compile(is);
		}
		else
			compile(std::cin);
		return 0;
	}
	catch(std::string &error)
	{
		std::cout << "Exception: " << error << std::endl;
		return 1;
	}
}

