#pragma once

#include <iostream>
#include <llvm/Function.h>
#include <vector>

class Engine;
class WordIndex;
class Word;

class WordInstance
{
	Word *word;
	std::vector<llvm::Value *> outputs;
public:
	WordInstance(Word *_word);
	~WordInstance();

	Word *GetWord();
	
	void SetOutput(size_t index, llvm::Value *output);
	llvm::Value *GetOutput(size_t index);
	size_t GetOutputSize() { return outputs.size(); }

	void Compile(Engine *e);
};

class Word
{
	bool inlined;
public:
	Word() : inlined(false) { }

	virtual std::string GetName() = 0;

	virtual void Execute(Engine* e, bool compiling) = 0;
	virtual void Compile(Engine *e, WordInstance *instance) = 0;

	bool IsInline() { return inlined; }
	void SetInline(bool inlined) { this->inlined = inlined; }
};

class WordIndex
{
	WordInstance *word_instance;
	size_t index;
public:
	WordIndex(WordInstance *word_instance, size_t index);

	WordInstance *GetWordInstance();
	size_t GetIndex();
	llvm::Value *GetOutput();
};

