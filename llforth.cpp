#include <iostream>
#include <fstream>
#include <llvm/Module.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <unistd.h>
#include "lexer.h"
#include "ast.h"
#include "parser.h"

using namespace llvm;
using namespace std;

static bool verbose = false;
static std::string output_filename("a.obj");
static bool optimize = false;

void show_help()
{
	cout << "llforth [OPTIONS]..." << endl << endl;
	cout << "  -h         	show help" << endl;
	cout << "  -v         	verbose output" << endl;
	cout << "  -o filename	object filename" << endl;
	cout << "  -O         	run optimize passes" << endl;
	exit(0);
}

void read_args(int argc, char **argv)
{
	int c;
	extern char *optarg;
	extern int optopt;

	while((c = getopt(argc, argv, "vho:O")) != -1)
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
		case '?':
			cerr << "Unknown option -" << (char)optopt << endl;
		}
}

void compile()
{
	std::ifstream in("test.llfs");
	Lexer lexer(in);
	lexer.NextToken();

	Parser parser(lexer);
	parser.MainLoop();

	// create llvm module
	Module module("llforth");
	parser.Compile(&module);

	if(optimize)
		parser.Optimize(&module);

	if(verbose)
	{
		parser.Print();
		module.dump();
	}

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
		compile();
		return 0;
	}
	catch(std::string &error)
	{
		std::cout << "Exception: " << error << std::endl;
		return 1;
	}
}

