#pragma once

#include "word.h"

class ColonWord : public Word
{
public:
	std::string GetName() { return ":"; }
	size_t GetInputSize() { return 0; }
	size_t GetOutputSize() { return 0; }
	void Execute(Engine* e);
	void Compile(Engine* e, WordInstance *instance) { throw std::string("not supported"); }
};

class PrintStackWord : public Word
{
public:
	std::string GetName() { return ".s"; }
	size_t GetInputSize() { return 0; }
	size_t GetOutputSize() { return 0; }
	void Execute(Engine* e);
	void Compile(Engine* e, WordInstance *instance) { throw std::string("not supported"); }
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
	size_t GetInputSize() { return inputs; }
	size_t GetOutputSize() { return outputs; }

	void Execute(Engine* e);
	void Compile(Engine *e, WordInstance *instance);
};

class LiteralWord : public Word
{
	int number;
public:
	LiteralWord(int _number) : number(_number) { }

	std::string GetName() { return "lit"; }
	size_t GetInputSize() { return 0; }
	size_t GetOutputSize() { return 1; }

	void Execute(Engine* e);
	void Compile(Engine* e, WordInstance *instance);
};

class ArgumentWord : public Word
{
	int number;
public:
	ArgumentWord(int _number) : number(_number) { }

	std::string GetName() { return "arg"; }
	size_t GetInputSize() { return 0; }
	size_t GetOutputSize() { return 1; }

	void Execute(Engine* e) { assert(false); }
	void Compile(Engine* e, WordInstance *instance);
};

class InlineWord : public Word
{
public:
	std::string GetName() { return "inline"; }
	size_t GetInputSize() { return 0; }
	size_t GetOutputSize() { return 0; }

	void Execute(Engine* e);
	void Compile(Engine* e, WordInstance *instance) { Execute(e); }
};

