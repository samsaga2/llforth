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
	std::vector<WordIndex *> inputs;
	llvm::Value **outputs;
public:
	WordInstance(Word *_word);
	~WordInstance();

	Word *GetWord();
	
	void SetInput(size_t index, WordIndex *word_index);
	WordIndex *GetInput(size_t index);
	void SetOutput(size_t index, llvm::Value *output);
	llvm::Value *GetOutput(size_t index);

	void Compile(Engine *e);
};

class Word
{
	bool inlined;
public:
	Word() : inlined(false) { }

	virtual std::string GetName() = 0;

	virtual void Execute(Engine* e) = 0;
	virtual void Compile(Engine *e, WordInstance *instance) = 0;

	virtual size_t GetInputSize() = 0;
	virtual size_t GetOutputSize() = 0;

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

