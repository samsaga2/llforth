#pragma once

#include <llvm/Support/IRBuilder.h>
#include <list>

using namespace llvm;

enum TypeAST
{
	TYPE_NULL,
	TYPE_INT32,
	TYPE_STRING,
	TYPE_ANY
};

class AST
{
public:
	AST();
	Value *GetValue(int index, IRBuilder<> builder);
	virtual TypeAST InputType(int index) = 0;
	virtual TypeAST OutputType(int index) = 0;
	virtual int InputSize() = 0;
	virtual int OutputSize() = 0;
	virtual void Print() = 0;
	virtual ~AST() { }
protected:
	bool compiled;
	std::vector<Value *> values;
	void SetValue(int index, Value *value);
	virtual void Compile(IRBuilder<> builder) = 0;
};

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
	void Compile(IRBuilder<> builder);
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
	void Compile(IRBuilder<> builder);
};

class BodyAST : public std::list<AST *>
{
public:
	int OutputSize();
	TypeAST OutputType(int index);
	void Print();
	AST *operator[](size_t index);
};

class FunctionAST
{
	std::string name;
	BodyAST *body;
	BodyAST *args;
	Function *function;
public:
	FunctionAST(const std::string &_name, BodyAST *_body, BodyAST *_args);
	const std::string Name();
	TypeAST InputType(int index);
	TypeAST OutputType(int index);
	int InputSize();
	int OutputSize();
	void Print();
	void Compile(Module *module);
	Function *CompiledFunction();
private:
	const Type *ConvertType(TypeAST t);
};

class CallAST : public AST
{
	FunctionAST *function;
	BodyAST *args;
public:
	CallAST(FunctionAST *_function, BodyAST *_args);
	TypeAST InputType(int index);
	TypeAST OutputType(int index);
	int InputSize();
	int OutputSize();
	void Print();
protected:
	void Compile(IRBuilder<> builder);
};

class ArgAST : public AST
{
	int n;
public:
	ArgAST(int _n);
	TypeAST InputType(int index);
	TypeAST OutputType(int index);
	int InputSize();
	int OutputSize();
	void Print();
protected:
	void Compile(IRBuilder<> builder);
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
	void Compile(IRBuilder<> builder);
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
	void Compile(IRBuilder<> builder);
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
	void Compile(IRBuilder<> builder);
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
	void Compile(IRBuilder<> builder);
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
	void Compile(IRBuilder<> builder);
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
	void Compile(IRBuilder<> builder);
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
	void Compile(IRBuilder<> builder);
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
	void Compile(IRBuilder<> builder);
};

