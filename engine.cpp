#include "engine.h"
#include "words.h"
#include "jit.h"
#include <sstream>

void word_dots()
{
	Engine &e = Engine::GetSingleton();
	for(std::list<int>::reverse_iterator it = e.runtime_stack.rbegin(); it != e.runtime_stack.rend(); it++)
		std::cout << "  " << *it;
	std::cout << std::endl;
}

void word_see()
{
	std::string word = Engine::GetSingleton().GetLexer()->NextWord();
	llvm::Function *function = JIT::GetSingleton().GetModule()->getFunction(word);
	if(function != NULL)
		function->dump();
}

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

	IWORD(".s", word_dots, 0, 0);
	IWORD("see", word_see, 0, 0);
	WORD(ColonWord);
	WORD(InlineWord);
	WORD(StringWord);
	WORD(ExternWord);

	BWORD("+");
		ARG(0);
		ARG(1);
		OUT(0, BUILDER->CreateAdd(arg0, arg1));
	EWORD();
	BWORD("-");
		ARG(0);
		ARG(1);
		OUT(0, BUILDER->CreateSub(arg0, arg1));
	EWORD();
	BWORD("*");
		ARG(0);
		ARG(1);
		OUT(0, BUILDER->CreateMul(arg0, arg1));
	EWORD();
	BWORD("/");
		ARG(0);
		ARG(1);
		OUT(0, BUILDER->CreateSDiv(arg0, arg1));
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
	BWORD("nip");
		ARG(0);
		ARG(1);
		OUT(0, arg0);
	EWORD();

#undef WORD
#undef BWORD
#undef JIT
#undef BUILDER
#undef ARG
#undef OUT
#undef EWORD
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
		if((*it)->GetName() == word)
			return *it;

	return NULL;
}

void Engine::ExecuteWord(const std::string &word)
{
	Word *w = FindWord(word);
	if(w != NULL)
	{
		w->Execute(this, false);
		return;
	}

	// integer?
	std::istringstream is(word);
	int number;
	if(is >> number)
	{
		LiteralWord lit(number);
		lit.Execute(this, false);
		return;
	}

	throw std::string("unknown word");
}

void Engine::CreateExternWord(const std::string &word, size_t inputs, size_t outputs)
{
	JIT::GetSingleton().CreateExternWord(word, inputs, outputs);

	latest = new FunctionWord(JIT::GetSingleton().GetLatest(), inputs, outputs);
	words.push_back(latest);
}

void Engine::CreateWord()
{
	JIT::GetSingleton().CreateWord();
	latest = NULL;

	compiler_stack.clear();
	compiler_args.clear();
}

void Engine::FinishWord(const std::string& word)
{
	size_t inputs = JIT::GetSingleton().GetInputSize();
	size_t outputs = JIT::GetSingleton().GetOutputSize();
	JIT::GetSingleton().FinishWord(word);

	latest = new FunctionWord(JIT::GetSingleton().GetLatest(), inputs, outputs);
	words.push_back(latest);
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
		arg_instance->Compile(this);
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

