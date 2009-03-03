#pragma once

#include <llvm/Support/IRBuilder.h>
#include <list>
#include "ast.h"
#include "function.h"

class IntegerAST : public AST
{
	int integer;
public:
	IntegerAST(int _integer);
	TypeAST InputType(int index);
	TypeAST OutputType(int index);
	int InputSize();
	int OutputSize();
	void Print();
protected:
	void DoCompile(IRBuilder<> builder);
};

class StringAST : public AST
{
	std::string str;
public:
	StringAST(const std::string &str);
	TypeAST InputType(int index);
	TypeAST OutputType(int index);
	int InputSize();
	int OutputSize();
	void Print();
protected:
	void DoCompile(IRBuilder<> builder);
};

class CallAST : public AST
{
	FunctionBaseAST *function;
	BodyAST *args;
public:
	CallAST(FunctionBaseAST *_function, BodyAST *_args);
	TypeAST InputType(int index);
	TypeAST OutputType(int index);
	int InputSize();
	int OutputSize();
	void Print();
protected:
	void DoCompile(IRBuilder<> builder);
};

class ArgAST : public AST
{
	int n;
	TypeAST type;
public:
	ArgAST(int _n, TypeAST _type);
	TypeAST InputType(int index);
	TypeAST OutputType(int index);
	int InputSize();
	int OutputSize();
	void Print();
protected:
	void DoCompile(IRBuilder<> builder);
};

class OutputIndexAST : public AST
{
	AST *ast;
	int index;
public:
	OutputIndexAST(AST *_ast, int _index);
	TypeAST InputType(int index);
	TypeAST OutputType(int index);
	int InputSize();
	int OutputSize();
	void Print();
protected:
	void DoCompile(IRBuilder<> builder);
};

class DupAST : public AST
{
	OutputIndexAST *arg1;
public:
	DupAST(OutputIndexAST *_arg1);
	TypeAST InputType(int index);
	TypeAST OutputType(int index);
	int InputSize();
	int OutputSize();
	void Print();
protected:
	void DoCompile(IRBuilder<> builder);
};

class MultAST : public AST
{
	OutputIndexAST *arg1;
	OutputIndexAST *arg2;
public:
	MultAST(OutputIndexAST *_arg1, OutputIndexAST *_arg2);
	TypeAST InputType(int index);
	TypeAST OutputType(int index);
	int InputSize();
	int OutputSize();
	void Print();
protected:
	void DoCompile(IRBuilder<> builder);
};

class AddAST : public AST
{
	OutputIndexAST *arg1;
	OutputIndexAST *arg2;
public:
	AddAST(OutputIndexAST *_arg1, OutputIndexAST *_arg2);
	TypeAST InputType(int index);
	TypeAST OutputType(int index);
	int InputSize();
	int OutputSize();
	void Print();
protected:
	void DoCompile(IRBuilder<> builder);
};

class SwapAST : public AST
{
	OutputIndexAST *arg1;
	OutputIndexAST *arg2;
public:
	SwapAST(OutputIndexAST *_arg1, OutputIndexAST *_arg2);
	TypeAST InputType(int index);
	TypeAST OutputType(int index);
	int InputSize();
	int OutputSize();
	void Print();
protected:
	void DoCompile(IRBuilder<> builder);
};

class OverAST : public AST
{
	OutputIndexAST *arg1;
	OutputIndexAST *arg2;
public:
	OverAST(OutputIndexAST *_arg1, OutputIndexAST *_arg2);
	TypeAST InputType(int index);
	TypeAST OutputType(int index);
	int InputSize();
	int OutputSize();
	void Print();
protected:
	void DoCompile(IRBuilder<> builder);
};

class RotAST : public AST
{
	OutputIndexAST *arg1;
	OutputIndexAST *arg2;
	OutputIndexAST *arg3;
public:
	RotAST(OutputIndexAST *_arg1, OutputIndexAST *_arg2, OutputIndexAST *_arg3);
	TypeAST InputType(int index);
	TypeAST OutputType(int index);
	int InputSize();
	int OutputSize();
	void Print();
protected:
	void DoCompile(IRBuilder<> builder);
};

class DropAST : public AST
{
	OutputIndexAST *arg1;
public:
	DropAST(OutputIndexAST *_arg1);
	TypeAST InputType(int index);
	TypeAST OutputType(int index);
	int InputSize();
	int OutputSize();
	void Print();
protected:
	void DoCompile(IRBuilder<> builder);
};

