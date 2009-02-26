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

// --- Lexer ---

class Lexer
{
public:
	enum Token
	{
		tok_eof = -1,
		tok_word = -2,
		tok_integer = -3
	};

	std::istream &in;
	int token;
	std::string word;
	int integer;

	Lexer(std::istream &_in) : in(_in) { }

	void NextToken()
	{
		if(in.eof())
			token = tok_eof;
		else
		{
			in >> word; 

			std::istringstream iss(word);
			if(iss >> integer)
				token = tok_integer;
			else
				token = tok_word;
		}
	}
};

// --- AST ---

class AST
{
public:
	virtual int InputSize() = 0;
	virtual int OutputSize() = 0;
	virtual void Print() = 0;
	virtual ~AST() { }
};

class IntegerAST : public AST
{
	int integer;
public:
	IntegerAST(int _integer)
		: integer(_integer) { }
	int InputSize() { return 0; }
	int OutputSize() { return 1; }
	void Print() { std::cout << integer; }
};

class BodyAST : public std::vector<AST *>
{
public:
	int OutputSize()
	{
		int output_size = 0;
		for(BodyAST::iterator it = this->begin(); it != this->end(); it++)
			output_size += (*it)->OutputSize();
		return output_size;
	}
	void Print()
	{
		for(BodyAST::iterator it = this->begin(); it != this->end(); it++)
		{
			std::cout << " ";
			(*it)->Print();
		}
	}
};

class FunctionAST : public AST
{
	std::string name;
	BodyAST *body;
	BodyAST *args;
public:
	FunctionAST(const std::string &_name, BodyAST *_body, BodyAST *_args)
		: name(_name), body(_body), args(_args) { }
	const std::string Name() { return name; }
	int InputSize() { return args->OutputSize(); }
	int OutputSize() { return body->OutputSize(); }
	void Print()
	{
		std::cout << name << " (";
		for(int i = 0; i < args->OutputSize(); i++)
			std::cout << " n";
		std::cout << " -- ";
		for(int i = 0; i < body->OutputSize(); i++)
			std::cout << " n";
		std::cout << " )" << std::endl;

		for(BodyAST::iterator it = body->begin(); it != body->end(); it++)
		{
			std::cout << " ";
			(*it)->Print();
		}
		
		std::cout << std::endl << std::endl;
	}
};

class CallAST : public AST
{
	FunctionAST *function;
	BodyAST *args;
public:
	CallAST(FunctionAST *_function, BodyAST *_args)
		: function(_function), args(_args) { }
	int InputSize() { return function->InputSize(); }
	int OutputSize() { return function->OutputSize(); }
	void Print() { std::cout << function->Name(); }
};

class ArgAST : public AST
{
	int n;
public:
	ArgAST(int _n) : n(_n) { }
	int InputSize() { return 0; }
	int OutputSize() { return 1; }
	void Print() { std::cout << "arg" << n; }
};

class DupAST : public AST
{
	BodyAST *args;
public:
	DupAST(BodyAST *_args) : args(_args)
	{
		assert(args->OutputSize() == 1);
	}
	int InputSize() { return 1; }
	int OutputSize() { return 2; }
	void Print() { args->Print(); std::cout << " dup"; }
};

class MultAST : public AST
{
	BodyAST *args;
public:
	MultAST(BodyAST *_args) : args(_args)
	{
		assert(args->OutputSize() == 2);
	}
	int InputSize() { return 2; }
	int OutputSize() { return 1; }
	void Print() { args->Print(); std::cout << " *"; }
};

// --- Parser ---

class InferenceStack
{
public:
	BodyAST stack;
	BodyAST args;

	InferenceStack() { }

	void Clear()
	{
		stack.clear();
		args.clear();
	}

	BodyAST *Pop(int size)
	{
		int stack_size = stack.OutputSize();

		if(stack_size < size)
			for(int i = 0; i < size - stack_size; i++)
			{
				AST *arg = new ArgAST(i + args.OutputSize());
				args.push_back(arg);
				stack.push_back(arg);
			}		

		BodyAST *func_args = new BodyAST();
		while(size > 0)
		{
			func_args->push_back(stack.back());
			size -= stack.back()->OutputSize();
			stack.pop_back();
		}
		return func_args;
	}

	void Push(AST *ast)
	{
		stack.push_back(ast);
	}
};

class Parser
{
public:
	Lexer lexer;
	typedef std::list<FunctionAST *> Functions;
	Functions functions;
	InferenceStack istack;

	Parser(Lexer &_lexer) : lexer(_lexer) { }

	FunctionAST *FindFunction(const std::string &word)
	{
		Functions::reverse_iterator it = functions.rbegin();
		while(it != functions.rend())
		{
			FunctionAST *function = *it;
			if(function->Name() == word)
				return function;
			it++;
		}
		return NULL;
	}

	void AppendFunction()
	{
		std::string func_name = lexer.word;
		lexer.NextToken();

		FunctionAST *function = FindFunction(func_name);
		assert(function != NULL);

		AST *call = new CallAST(function, istack.Pop(function->InputSize()));
		istack.Push(call);
	}

	AST *AppendCore()
	{
		std::string word = lexer.word;
		lexer.NextToken();

		if(word == "dup")
		{
			AST *dup = new DupAST(istack.Pop(1));
			istack.Push(dup);
		}
		else if(word == "*")
		{
			AST *mult = new MultAST(istack.Pop(2));
			istack.Push(mult);
		}
		else
		{
			std::string error("unknown token `");
			error += lexer.word;
			error += "'";
			throw error;
		}
	}

	void AppendInteger()
	{
		int integer = lexer.integer;
		lexer.NextToken();

		AST *ast = new IntegerAST(integer);
		istack.Push(ast);
	}

	void ParseWordExpr()
	{
		FunctionAST *function = FindFunction(lexer.word);
		if(function != NULL)
			AppendFunction();
		else
			switch(lexer.token)
			{
			default:
				throw std::string("end of file");
			case Lexer::tok_word:
				AppendCore();
				break;
			case Lexer::tok_integer:
				AppendInteger();
				break;
			}
	}

	void ParseBody(const std::string &end)
	{
		istack.Clear();
		do
		{
			if(lexer.word == end)
				break;

			ParseWordExpr();
		}
		while(true);
	}

	FunctionAST *ParseFunction()
	{
		assert(lexer.word == ":");
		lexer.NextToken();

		std::string func_name = lexer.word;
		lexer.NextToken();

		ParseBody(";");

		BodyAST *func_body = new BodyAST();
		for(BodyAST::iterator it = istack.stack.begin(); it != istack.stack.end(); it++)
			func_body->push_back(*it);

		BodyAST *func_args = new BodyAST();
		for(BodyAST::iterator it = istack.args.begin(); it != istack.args.end(); it++)
			func_args->push_back(*it);

		return new FunctionAST(func_name, func_body, func_args);
	}

	void MainLoop()
	{
		while(true)
		{
			if(lexer.token == Lexer::tok_eof)
				break;
			else
			{
				FunctionAST *function = ParseFunction();
				functions.push_back(function);
				function->Print();
				lexer.NextToken();
			}
		}
	}
};

int main(int argc, char **argv)
{
	try
	{
		std::istringstream in(": double dup * ;");
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

