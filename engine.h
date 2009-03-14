#pragma once

#include <list>
#include "lexer.h"
#include "words.h"

class Engine
{
	bool verbose;

	Lexer *lexer;

	typedef std::list<Word *> Words;
	Words words;
	FunctionWord *latest;

	Engine();
public:
	~Engine();

	static Engine &GetSingleton();

	std::list<int> runtime_stack;
	std::list<WordIndex *> compiler_stack;
	std::list<ArgumentWord *> compiler_args;

	void SetInputStream(std::istream &in);
	void SetVerbose(bool verbose) { this->verbose = verbose; }
	bool GetVerbose() { return verbose; }
	Lexer *GetLexer() { return lexer; }
	FunctionWord *GetLatest() { return latest; }

	void MainLoop();
	Word *FindWord(const std::string& word);
	void ExecuteWord(const std::string& word);

	void CreateExternWord(const std::string &word, size_t inputs, size_t outputs);
	void CreateWord();
	void FinishWord(const std::string& word);
	void Push(WordInstance *instance);
	WordIndex *Pop();
};

