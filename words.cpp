#include "words.h"
#include "engine.h"
#include <sstream>
#include <iostream>
#include <llvm/ExecutionEngine/GenericValue.h>

void ColonWord::Execute(Engine* e)
{
	std::string function_name = e->GetLexer()->NextToken();
	
	// create function
	e->CreateWord();

	// read body
	std::list<WordIndex *> stack;
	std::list<ArgumentWord *> args;
	while(true)
	{
		std::string token = e->GetLexer()->NextToken();
		if(token == ";")
			break;

		// find word
		Word *word = e->FindWord(token);
		if(word == NULL)
		{
			// integer?
			std::istringstream is(token);
			int number;
			if(is >> number)
				word = new LiteralWord(number);
		}
		else if(word->IsInline())
		{
			// inline
			word->Execute(e);
			continue;
		}

		assert(word != NULL);

		// create word instance
		WordInstance *instance = new WordInstance(word);

		// setup inputs
		for(size_t i = 0; i < word->GetInputSize(); i++)
		{
			WordIndex *value;
			if(stack.size() == 0)
			{
				// drop value from function argument
				ArgumentWord *arg = new ArgumentWord(args.size());
				WordInstance *arg_instance = new WordInstance(arg);
				args.push_back(arg);
				arg_instance->Compile(e);
				value = new WordIndex(arg_instance, 0);
			}
			else
			{
				// drop value from stack
				value = stack.back();
				stack.pop_back();
			}
			instance->SetInput(i, value);
		}

		instance->Compile(e);

		// setup outputs
		for(size_t i = 0; i < word->GetOutputSize(); i++)
			stack.push_back(new WordIndex(instance, i));
	}

	// print word info
	if(e->GetVerbose())
		std::cerr << "WORD: " << function_name << " ins:" << args.size() << " outs:" << stack.size() << std::endl;

	// setup outputs
	llvm::Function *latest = e->GetJIT()->GetLatest();
	for(size_t i = 0; !stack.empty(); i++)
	{
		llvm::Value *input = stack.back()->GetOutput();
		llvm::Value *output = e->GetJIT()->CreateOutputArgument();

		e->GetJIT()->GetBuilder()->CreateStore(input, output);
		stack.pop_back();
	}

	e->GetJIT()->GetBuilder()->CreateRetVoid();
	e->FinishWord(function_name);

	if(e->GetVerbose())
		e->GetJIT()->GetLatest()->dump();
}

void PrintStackWord::Execute(Engine* e)
{
	for(std::list<int>::reverse_iterator it = e->stack.rbegin(); it != e->stack.rend(); it++)
		std::cout << "  " << *it;
	std::cout << std::endl;
}

FunctionWord::FunctionWord(llvm::Function *_function, size_t _inputs, size_t _outputs) : function(_function), inputs(_inputs), outputs(_outputs)
{
}

void FunctionWord::Execute(Engine* e)
{
	// setup inputs
	std::vector<llvm::GenericValue> arguments(inputs + outputs);
	for(size_t i = 0; i < inputs; i++)
	{
		int number = e->stack.front();
		arguments[i].IntVal = llvm::APInt(32, number);
		e->stack.pop_front();
	}

	// setup outputs
	int outs[outputs];
	for(size_t i = 0; i < outputs; i++)
		arguments[i + inputs].PointerVal = (llvm::PointerTy)&outs[i];

	e->GetJIT()->GetExecutionEngine()->runFunction(function, arguments);

	// push outs
	for(size_t i = 0; i < outputs; i++)
		e->stack.push_front(outs[i]);
}

void FunctionWord::Compile(Engine *e, WordInstance *instance)
{
	// setup inputs
	std::vector<llvm::Value *> arguments(inputs + outputs);
	for(size_t i = 0; i < inputs; i++)
		arguments[i] = instance->GetInput(i)->GetOutput();

	// setup outputs
	for(size_t i = 0; i < outputs; i++)
	{
		llvm::Value *value = e->GetJIT()->GetBuilder()->CreateAlloca(llvm::Type::Int32Ty);
		arguments[i + inputs] = value;
		instance->SetOutput(i, value);
	}

	// append call
	e->GetJIT()->GetBuilder()->CreateCall<std::vector<llvm::Value *>::iterator>(function, arguments.begin(), arguments.end());

	// finish outputs
	for(size_t i = 0; i < outputs; i++)
	{
		llvm::Value *output = instance->GetOutput(i);
		output = e->GetJIT()->GetBuilder()->CreateLoad(output);
		instance->SetOutput(i, output);
	}
}

void LiteralWord::Execute(Engine* e)
{
	e->stack.push_front(number);
}

void LiteralWord::Compile(Engine* e, WordInstance *instance)
{
	llvm::Value *output = llvm::ConstantInt::get(llvm::APInt(32, number));
	instance->SetOutput(0, output);
}

void ArgumentWord::Compile(Engine* e, WordInstance *instance)
{
	llvm::Value *output = e->GetJIT()->CreateInputArgument();
	instance->SetOutput(0, output);
}

void InlineWord::Execute(Engine* e)
{
	e->GetLatest()->SetInline(true);
}

void SeeWord::Execute(Engine* e)
{
	std::string word = e->GetLexer()->NextWord();
	llvm::Function *function = e->GetJIT()->GetModule()->getFunction(word);
	if(function != NULL)
		function->dump();
}

