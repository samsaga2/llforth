#pragma once

#include <list>
#include "lexer.h"
#include "words.h"
#include "jit.h"

class Engine
{
	bool verbose;

	JIT jit;
	Lexer lexer;

	typedef std::list<Word *> Words;
	Words words;
	Word *latest;
public:
	std::list<int> runtime_stack;
	std::list<WordIndex *> stack;
	std::list<ArgumentWord *> args;

	Engine(std::istream &in);

	void SetOptimize(bool optimize) { jit.SetOptimize(optimize); }
	void SetVerbose(bool verbose) { this->verbose = verbose; }
	bool GetVerbose() { return verbose; }

	JIT *GetJIT() { return &jit; }
	Lexer *GetLexer() { return &lexer; }
	Word *GetLatest() { return latest; }

	void MainLoop();
	Word *FindWord(const std::string& word);
	void ExecuteWord(const std::string& word);

	void CreateWord();
	void FinishWord(const std::string& word);
	WordIndex *Pop();
};

