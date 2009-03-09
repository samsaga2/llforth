#include "engine.h"
#include "words.h"
#include "jit.h"
#include <sstream>

#include "words.inc"

Engine::Engine()
{
	verbose = false;
	latest = NULL;
	lexer = new Lexer(std::cin);

#define WORD(name) words.push_back(new name())
#define BWORD(name) CreateWord(); { std::string _name = name
#define BUILDER JIT::GetSingleton().GetBuilder()
#define ARG(number) llvm::Value *arg##number = JIT::GetSingleton().CreateInputArgument()
#define OUT(number, val) BUILDER->CreateStore(val, JIT::GetSingleton().CreateOutputArgument())
#define EWORD() BUILDER->CreateRetVoid(); FinishWord(_name); }
#define IWORD(name, func, inputs, outputs) \
	JIT::GetSingleton().AddInternalSymbol(name, (void *)&func); \
	CreateExternWord(name, inputs, outputs);
#define INLINE() latest->SetInline(true)

	#include "words_declare.inc"

#undef WORD
#undef BWORD
#undef JIT
#undef BUILDER
#undef ARG
#undef OUT
#undef EWORD
#undef IWORD
#undef INLINE
}

Engine::~Engine()
{
	delete lexer;
}

void Engine::SetInputStream(std::istream &in)
{
	delete lexer;
	lexer = new Lexer(in);
}

Engine &Engine::GetSingleton()
{
	static Engine engine;
	return engine;
}

void Engine::MainLoop()
{
	try
	{
		while(true)
		{
			std::string word = lexer->NextWord();
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
		if(!(*it)->IsHidden() && (*it)->GetName() == word)
			return *it;

	return NULL;
}

void Engine::ExecuteWord(const std::string &word)
{
	Word *w = FindWord(word);
	if(w != NULL)
	{
		w->Execute(false);
		return;
	}

	// integer?
	std::istringstream is(word);
	int number;
	if(is >> number)
	{
		LiteralWord lit(number);
		lit.Execute(false);
		return;
	}

	throw std::string("unknown word");
}

void Engine::CreateExternWord(const std::string &word, size_t inputs, size_t outputs)
{
	JIT::GetSingleton().CreateExternWord(word, inputs, outputs);

	latest = new FunctionWord();
	latest->SetFunction(JIT::GetSingleton().GetLatest());
	latest->SetInputSize(inputs);
	latest->SetOutputSize(outputs);
	words.push_back(latest);
}

void Engine::CreateWord()
{
	JIT::GetSingleton().CreateWord();

	compiler_stack.clear();
	compiler_args.clear();

	latest = new FunctionWord();
	latest->SetHidden(true);
	words.push_back(latest);
}

void Engine::FinishWord(const std::string& word)
{
	latest->SetInputSize(JIT::GetSingleton().GetInputSize());
	latest->SetOutputSize(JIT::GetSingleton().GetOutputSize());

	JIT::GetSingleton().FinishWord(word);
	latest->SetFunction(JIT::GetSingleton().GetLatest());
	latest->SetHidden(false);
}

void Engine::Push(WordInstance *instance)
{
	// setup outputs
	for(size_t i = 0; i < instance->GetOutputSize(); i++)
		compiler_stack.push_back(new WordIndex(instance, i));
}

WordIndex *Engine::Pop()
{
	WordIndex *value;
	if(compiler_stack.size() == 0)
	{
		// drop value from function argument
		ArgumentWord *arg = new ArgumentWord(compiler_args.size());
		WordInstance *arg_instance = new WordInstance(arg);
		compiler_args.push_back(arg);
		arg_instance->Compile();
		value = new WordIndex(arg_instance, 0);
	}
	else
	{
		// drop value from stack
		value = compiler_stack.back();
		compiler_stack.pop_back();
	}

	return value;
}

