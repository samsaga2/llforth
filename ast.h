#pragma once

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

class BodyAST : public std::list<AST *>
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
		std::cout << name << " ";
		for(int i = 0; i < args->OutputSize(); i++)
			std::cout << " n";
		std::cout << " -- ";
		for(int i = 0; i < body->OutputSize(); i++)
			std::cout << " n";

		for(BodyAST::iterator it = body->begin(); it != body->end(); it++)
		{
			std::cout << std::endl << "  ";
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

class OutputIndexAST : public AST
{
	AST *ast;
	int index;
public:
	OutputIndexAST(AST *_ast, int _index)
		: ast(_ast), index(_index) { }
	int InputSize() { return 0; }
	int OutputSize() { return 1; }
	void Print()
	{
		if(ast->OutputSize() == 1)
			ast->Print();
		else
		{
			std::cout << "[";
			ast->Print();
			std::cout << "]:" << index;
		}
	}
};

class DupAST : public AST
{
	OutputIndexAST *arg1;
public:
	DupAST(OutputIndexAST *_arg1) : arg1(_arg1) { }
	int InputSize() { return 1; }
	int OutputSize() { return 2; }
	void Print()
	{
		arg1->Print();
		std::cout << " ";
		arg1->Print();
	}
};

class MultAST : public AST
{
	OutputIndexAST *arg1;
	OutputIndexAST *arg2;
public:
	MultAST(OutputIndexAST *_arg1, OutputIndexAST *_arg2) : arg1(_arg1), arg2(_arg2) { }
	int InputSize() { return 2; }
	int OutputSize() { return 1; }
	void Print()
	{
		std::cout << "(";
		arg1->Print();
		std::cout << "*";
		arg2->Print();
		std::cout << ")";
	}
};

class AddAST : public AST
{
	OutputIndexAST *arg1;
	OutputIndexAST *arg2;
public:
	AddAST(OutputIndexAST *_arg1, OutputIndexAST *_arg2) : arg1(_arg1), arg2(_arg2) { }
	int InputSize() { return 2; }
	int OutputSize() { return 1; }
	void Print()
	{
		std::cout << "(";
		arg1->Print();
		std::cout << "+";
		arg2->Print();
		std::cout << ")";
	}
};

