#include <iostream>
#include <fstream>
#include <unistd.h>
#include <llvm/PassManager.h>
#include <llvm/CodeGen/Passes.h>
#include <llvm/LinkAllPasses.h>
#include <llvm/Target/TargetData.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/PassManager.h>
#include <llvm/CodeGen/Passes.h>
#include <llvm/LinkAllPasses.h>
#include <llvm/Target/TargetData.h>
#include "engine.h"

static bool verbose = false;
static std::string input_filename("");
static std::string output_filename("");
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
			std::cerr << "Unknown option -" << (char)optopt << std::endl;
		}
}

void compile(std::istream &in)
{
	Engine engine(in);
	engine.SetOptimize(optimize);
	engine.SetVerbose(verbose);
	engine.MainLoop();

	if(output_filename != "")
	{
		llvm::Module *module = engine.GetJIT()->GetModule();

		if(optimize)
		{
			llvm::PassManager pm;
			pm.add(new llvm::TargetData(module));
			pm.add(llvm::createIPSCCPPass());
			pm.add(llvm::createGlobalOptimizerPass());
			pm.add(llvm::createInstructionCombiningPass());
			pm.add(llvm::createFunctionInliningPass());
			pm.add(llvm::createGlobalOptimizerPass());
			pm.add(llvm::createInstructionCombiningPass());
			pm.add(llvm::createGVNPass());
			pm.add(llvm::createPromoteMemoryToRegisterPass());
			pm.add(llvm::createCFGSimplificationPass());
			pm.run(*module);
		}

		if(verbose)
			module->dump();

		// save object file
		std::ofstream of(output_filename.c_str(), std::ios::binary);
		llvm::WriteBitcodeToFile(module, of);
		of.close();
	}
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

