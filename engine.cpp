#include "engine.h"
#include "words.h"
#include <sstream>

#define WORD(name) words.push_back(new name())
#define BWORD(name) CreateWord(); { std::string _name = name
#define JIT jit.GetBuilder()
#define ARG(number) llvm::Value *arg##number = jit.CreateInputArgument()
#define OUT(number, val) JIT->CreateStore(val, jit.CreateOutputArgument())
#define EWORD() JIT->CreateRetVoid(); FinishWord(_name); }

Engine::Engine(std::istream &in) : lexer(in)
{
	verbose = false;
	latest = NULL;

	WORD(ColonWord);
	WORD(PrintStackWord);
	WORD(InlineWord);
	WORD(SeeWord);

	BWORD("+");
		ARG(0);
		ARG(1);
		OUT(0, JIT->CreateAdd(arg0, arg1));
	EWORD();
	BWORD("-");
		ARG(0);
		ARG(1);
		OUT(0, JIT->CreateSub(arg0, arg1));
	EWORD();
	BWORD("*");
		ARG(0);
		ARG(1);
		OUT(0, JIT->CreateMul(arg0, arg1));
	EWORD();
	BWORD("/");
		ARG(0);
		ARG(1);
		OUT(0, JIT->CreateSDiv(arg0, arg1));
	EWORD();
	BWORD("drop");
		ARG(0);
	EWORD();
	BWORD("dup");
		ARG(0);
		OUT(0, arg0);
		OUT(1, arg0);
	EWORD();
	BWORD("over");
		ARG(0);
		ARG(1);
		OUT(0, arg1);
		OUT(1, arg0);
		OUT(2, arg1);
	EWORD();
	BWORD("rot");
		ARG(0);
		ARG(1);
		ARG(2);
		OUT(0, arg1);
		OUT(1, arg0);
		OUT(2, arg2);
	EWORD();
	BWORD("swap");
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

void Engine::CreateWord()
{
	jit.CreateWord();
	latest = NULL;
}

void Engine::FinishWord(const std::string& word)
{
	size_t inputs = jit.GetInputSize();
	size_t outputs = jit.GetOutputSize();
	jit.FinishWord(word);

	latest = new FunctionWord(jit.GetLatest(), inputs, outputs);
	words.push_back(latest);
}

