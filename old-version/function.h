#pragma once

#include <llvm/Support/IRBuilder.h>
#include <list>
#include "ast.h"

class FunctionBaseAST
{
public:
	virtual const std::string Name() = 0;
	virtual TypeAST InputType(int index) = 0;
	virtual TypeAST OutputType(int index) = 0;
	virtual int InputSize() = 0;
	virtual int OutputSize() = 0;
	virtual void Print() = 0;
	virtual void Compile(Module *module) = 0;
	virtual Function *CompiledFunction() = 0;
};

class FunctionAST : public FunctionBaseAST
{
	std::string name;
	BodyAST *body;
	std::list<ArgAST *> *args;
	std::list<OutputIndexAST *> *outputs;
	Function *function;
public:
	FunctionAST(const std::string &_name, BodyAST *_body, std::list<ArgAST *> *_args, std::list<OutputIndexAST *> *_outputs);
	const std::string Name();
	TypeAST InputType(int index);
	TypeAST OutputType(int index);
	int InputSize();
	int OutputSize();
	void Print();
	void Compile(Module *module);
	Function *CompiledFunction();
};

class ExternAST : public FunctionBaseAST
{
	std::string name;
	std::vector<TypeAST> inputs;
	std::vector<TypeAST> outputs;
	Function *function;
public:
	ExternAST(const std::string &_name, std::vector<TypeAST> _inputs, std::vector<TypeAST> _outputs);
	const std::string Name();
	TypeAST InputType(int index);
	TypeAST OutputType(int index);
	int InputSize();
	int OutputSize();
	void Print();
	void Compile(Module *module);
	Function *CompiledFunction();
};

