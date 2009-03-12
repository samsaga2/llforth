#pragma once

#include "word.h"

class ColonWord : public Word
{
public:
	std::string GetName() { return ":"; }
	void Execute(Engine* e, WordInstance *instance);
};

class FunctionWord : public Word
{
	llvm::Function *function;
	size_t inputs;
	size_t outputs;
public:
	FunctionWord(llvm::Function *_function, size_t _inputs, size_t _outputs);

	std::string GetName() { return function->getName(); }
	llvm::Function *GetFunction() { return function; }

	void Execute(Engine* e, WordInstance *instance);
};

class LiteralWord : public Word
{
	int number;
public:
	LiteralWord(int _number) : number(_number) { }

	std::string GetName() { return "lit"; }

	void Execute(Engine* e, WordInstance *instance);
};

class ArgumentWord : public Word
{
	int number;
public:
	ArgumentWord(int _number) : number(_number) { }

	std::string GetName() { return "arg"; }

	void Execute(Engine* e, WordInstance *instance);
};

class InlineWord : public Word
{
public:
	std::string GetName() { return "inline"; }

	void Execute(Engine* e, WordInstance *instance);
};

class StringWord : public Word
{
public:
	std::string GetName() { return "s\""; }

	void Execute(Engine* e, WordInstance *instance);
};

class ExternWord : public Word
{
public:
	std::string GetName() { return "extern"; }

	void Execute(Engine* e, WordInstance *instance);
};

