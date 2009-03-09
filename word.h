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

	void Compile();
};

class Word
{
	bool inlined;
	bool hidden;
public:
	Word() : inlined(false), hidden(false) { }

	virtual std::string GetName() = 0;
	bool IsInline() { return inlined; }
	bool IsHidden() { return hidden; }
	void SetInline(bool inlined) { this->inlined = inlined; }
	void SetHidden(bool hidden) { this->hidden = hidden; }

	virtual void Execute(WordInstance *instance) = 0;
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

