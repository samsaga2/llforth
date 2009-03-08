#include "words.h"
#include "engine.h"
#include <sstream>
#include <iostream>
#include <llvm/ExecutionEngine/GenericValue.h>

void ColonWord::Execute(Engine* e, bool compiling)
{
	if(compiling)
		assert(false);

	std::string function_name = e->GetLexer()->NextToken();
	
	// create function
	e->CreateWord();

	// read body
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
			word->Execute(e, true);
			continue;
		}

		assert(word != NULL);

		// create word instance
		WordInstance *instance = new WordInstance(word);
		instance->Compile(e);

		e->Push(instance);
	}

	// print word info
	if(e->GetVerbose())
		std::cerr << "WORD: " << function_name << " ins:" << e->compiler_args.size() << " outs:" << e->compiler_stack.size() << std::endl;

	// setup outputs
	llvm::Function *latest = e->GetJIT()->GetLatest();
	for(size_t i = 0; !e->compiler_stack.empty(); i++)
	{
		llvm::Value *input = e->compiler_stack.back()->GetOutput();
		llvm::Value *output = e->GetJIT()->CreateOutputArgument();

		e->GetJIT()->GetBuilder()->CreateStore(input, output);
		e->compiler_stack.pop_back();
	}

	e->GetJIT()->GetBuilder()->CreateRetVoid();
	e->FinishWord(function_name);

	if(e->GetVerbose())
		e->GetJIT()->GetLatest()->dump();
}

void PrintStackWord::Execute(Engine* e, bool compiling)
{
	if(compiling)
		assert(false);

	for(std::list<int>::reverse_iterator it = e->runtime_stack.rbegin(); it != e->runtime_stack.rend(); it++)
		std::cout << "  " << *it;
	std::cout << std::endl;
}

FunctionWord::FunctionWord(llvm::Function *_function, size_t _inputs, size_t _outputs) : function(_function), inputs(_inputs), outputs(_outputs)
{
}

void FunctionWord::Execute(Engine* e, bool compiling)
{
	// setup inputs
	std::vector<llvm::GenericValue> arguments(inputs + outputs);
	for(size_t i = 0; i < inputs; i++)
	{
		int number = e->runtime_stack.front();
		arguments[i].IntVal = llvm::APInt(32, number);
		e->runtime_stack.pop_front();
	}

	// setup outputs
	int outs[outputs];
	for(size_t i = 0; i < outputs; i++)
		arguments[i + inputs].PointerVal = (llvm::PointerTy)&outs[i];

	e->GetJIT()->GetExecutionEngine()->runFunction(function, arguments);

	// push outs
	for(size_t i = 0; i < outputs; i++)
		e->runtime_stack.push_front(outs[i]);
}

void FunctionWord::Compile(Engine *e, WordInstance *instance)
{
	// setup inputs
	std::vector<llvm::Value *> arguments(inputs + outputs);
	for(size_t i = 0; i < inputs; i++)
	{
		WordIndex *input = e->Pop();
		arguments[i] = input->GetOutput();
	}

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

void LiteralWord::Execute(Engine* e, bool compiling)
{
	e->runtime_stack.push_front(number);
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

void InlineWord::Execute(Engine* e, bool compiling)
{
	e->GetLatest()->SetInline(true);
}

void InlineWord::Compile(Engine* e, WordInstance *instance)
{
	e->GetLatest()->SetInline(true);
}

void SeeWord::Execute(Engine* e, bool compiling)
{
	if(compiling)
		assert(false);

	std::string word = e->GetLexer()->NextWord();
	llvm::Function *function = e->GetJIT()->GetModule()->getFunction(word);
	if(function != NULL)
		function->dump();
}

