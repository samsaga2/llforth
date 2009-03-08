#include "engine.h"
#include "words.h"
#include <sstream>

#define WORD(name) words.push_back(new name())
#define BWORD(name, inputs, outputs) CreateWord(name, inputs, outputs); { int _inputs = inputs
#define JIT jit.GetBuilder()
#define ARG(number) llvm::Value *arg##number = jit.GetArgument(number)
#define OUT(number, val) JIT->CreateStore(val, jit.GetArgument(_inputs + number))
#define EWORD() JIT->CreateRetVoid(); } FinishWord()

Engine::Engine(std::istream &in) : lexer(in)
{
	verbose = false;
	latest = NULL;

	WORD(ColonWord);
	WORD(PrintStackWord);
	WORD(InlineWord);
	WORD(SeeWord);

	BWORD("+", 2, 1);
		ARG(0);
		ARG(1);
		OUT(0, JIT->CreateAdd(arg0, arg1));
	EWORD();
	BWORD("-", 2, 1);
		ARG(0);
		ARG(1);
		OUT(0, JIT->CreateSub(arg0, arg1));
	EWORD();
	BWORD("*", 2, 1);
		ARG(0);
		ARG(1);
		OUT(0, JIT->CreateMul(arg0, arg1));
	EWORD();
	BWORD("/", 2, 1);
		ARG(0);
		ARG(1);
		OUT(0, JIT->CreateSDiv(arg0, arg1));
	EWORD();
	BWORD("drop", 1, 0);
	EWORD();
	BWORD("dup", 1, 2);
		ARG(0);
		OUT(0, arg0);
		OUT(1, arg0);
	EWORD();
	BWORD("over", 2, 3);
		ARG(0);
		ARG(1);
		OUT(0, arg1);
		OUT(1, arg0);
		OUT(2, arg1);
	EWORD();
	BWORD("rot", 3, 3);
		ARG(0);
		ARG(1);
		ARG(2);
		OUT(0, arg1);
		OUT(1, arg0);
		OUT(2, arg2);
	EWORD();
	BWORD("swap", 2, 2);
		ARG(0);
		ARG(1);
		OUT(0, arg0);
		OUT(1, arg1);
	EWORD();
}

void Engine::MainLoop()
{
	try
	{
		while(true)
		{
			std::string word = lexer.NextWord();
			ExecuteWord(word);
		}
	}
	catch(EndOfStream &eof)
	{
	}
}

Word *Engine::FindWord(const std::string &word)
{
	for(Words::reverse_iterator it = words.rbegin(); it != words.rend(); it++)
		if((*it)->GetName() == word)
			return *it;

	return NULL;
}

void Engine::ExecuteWord(const std::string &word)
{
	Word *w = FindWord(word);
	if(w != NULL)
	{
		w->Execute(this);
		return;
	}

	// integer?
	std::istringstream is(word);
	int number;
	if(is >> number)
	{
		LiteralWord lit(number);
		lit.Execute(this);
		return;
	}

	throw std::string("unknown word");
}

void Engine::CreateWord(const std::string& word, size_t inputs, size_t outputs)
{
	jit.CreateWord(word, inputs, outputs);
	latest = new FunctionWord(jit.GetLatest(), inputs, outputs);
}

void Engine::FinishWord()
{
	jit.FinishWord();
	words.push_back(latest);
}

