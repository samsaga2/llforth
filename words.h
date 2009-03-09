#pragma once

#include "word.h"

class FunctionWord : public Word
{
	llvm::Function *function;
	size_t inputs;
	size_t outputs;
public:
	FunctionWord();

	std::string GetName() { return function->getName(); }
	llvm::Function *GetFunction() { return function; }
	void SetFunction(llvm::Function* function) { this->function = function; }
	size_t GetInputSize() { return inputs; }
	void SetInputSize(size_t inputs) { this->inputs = inputs; }
	size_t GetOutputSize() { return outputs; }
	void SetOutputSize(size_t outputs) { this->outputs = outputs; }

	void Execute(WordInstance *instance);
};

class LiteralWord : public Word
{
	int number;
public:
	LiteralWord(int _number) : number(_number) { }

	std::string GetName() { return "lit"; }

	void Execute(WordInstance *instance);
};

class ArgumentWord : public Word
{
	int number;
public:
	ArgumentWord(int _number) : number(_number) { }

	std::string GetName() { return "arg"; }

	void Execute(WordInstance *instance);
};

class StringWord : public Word
{
public:
	std::string GetName() { return "s\""; }

	void Execute(WordInstance *instance);
};

