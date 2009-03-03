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
	void Compile(IRBuilder<> builder);
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
	virtual void DoCompile(IRBuilder<> builder) = 0;
};

class BodyAST : public std::list<AST *>
{
public:
	int OutputSize();
	TypeAST OutputType(int index);
	void Print();
	AST *operator[](size_t index);
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

class OutputIndexAST
{
	AST *ast;
	int index;
	Value *value;
	bool compiled;
public:
	OutputIndexAST(AST *_ast, int _index);
	TypeAST OutputType();
	void Print();
	Value *GetValue(IRBuilder<> builder);
	void Compile(IRBuilder<> builder);
};

typedef std::list<OutputIndexAST *> OutputList;
typedef std::list<ArgAST *> ArgumentsList;

