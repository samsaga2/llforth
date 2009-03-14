#include <sstream>
#include <iostream>
#include <llvm/ExecutionEngine/GenericValue.h>
#include "words.h"
#include "engine.h"
#include "jit.h"

void ColonWord::Execute(Engine* e, WordInstance *instance)
{
	assert(instance == NULL);

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
			word->Execute(e, NULL);
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
	llvm::Function *latest = JIT::GetSingleton().GetLatest();
	for(size_t i = 0; !e->compiler_stack.empty(); i++)
	{
		llvm::Value *input = e->compiler_stack.back()->GetOutput();
		llvm::Value *output = JIT::GetSingleton().CreateOutputArgument();

		JIT::GetSingleton().GetBuilder()->CreateStore(input, output);
		e->compiler_stack.pop_back();
	}

	JIT::GetSingleton().GetBuilder()->CreateRetVoid();
	e->FinishWord(function_name);

	if(e->GetVerbose())
		JIT::GetSingleton().GetLatest()->dump();
}

FunctionWord::FunctionWord() : function(NULL), inputs(0), outputs(0)
{
}

void FunctionWord::Execute(Engine* e, WordInstance *instance)
{
	size_t real_outputs;
	const llvm::FunctionType *ftype = function->getFunctionType();
	if(ftype->getReturnType() != llvm::Type::VoidTy)
		real_outputs = outputs - 1;
	else
		real_outputs = outputs;

	if(instance == NULL)
	{
		// setup inputs
		std::vector<llvm::GenericValue> arguments(inputs + real_outputs);
		for(size_t i = 0; i < inputs; i++)
		{
			int number = e->runtime_stack.front();
			arguments[i].IntVal = llvm::APInt(32, number);
			e->runtime_stack.pop_front();
		}

		// setup outputs
		int outs[real_outputs];
		for(size_t i = 0; i < real_outputs; i++)
			arguments[i + inputs].PointerVal = (llvm::PointerTy)&outs[i];

		llvm::GenericValue ret = JIT::GetSingleton().GetExecutionEngine()->runFunction(function, arguments);

		// push outs
		for(size_t i = 0; i < real_outputs; i++)
			e->runtime_stack.push_front(outs[i]);
		
		const llvm::FunctionType *ftype = function->getFunctionType();
		if(ftype->getReturnType() != llvm::Type::VoidTy)
			e->runtime_stack.push_front((int)ret.PointerVal);	
	}
	else
	{
		// setup inputs
		std::vector<llvm::Value *> arguments(inputs + real_outputs);
		for(size_t i = 0; i < inputs; i++)
		{
			WordIndex *input = e->Pop();
			arguments[i] = input->GetOutput();
		}

		// setup outputs
		for(size_t i = 0; i < real_outputs; i++)
		{
			llvm::Value *value = JIT::GetSingleton().GetBuilder()->CreateAlloca(llvm::Type::Int32Ty);
			arguments[i + inputs] = value;
			instance->SetOutput(i, value);
		}

		// append call
		JIT::GetSingleton().GetBuilder()->CreateCall<std::vector<llvm::Value *>::iterator>(function, arguments.begin(), arguments.end());

		// finish outputs
		for(size_t i = 0; i < real_outputs; i++)
		{
			llvm::Value *output = instance->GetOutput(i);
			output = JIT::GetSingleton().GetBuilder()->CreateLoad(output);
			instance->SetOutput(i, output);
		}
	}
}

void LiteralWord::Execute(Engine* e, WordInstance *instance)
{
	if(instance == NULL)
		e->runtime_stack.push_front(number);
	else
	{
		llvm::Value *output = llvm::ConstantInt::get(llvm::APInt(32, number));
		instance->SetOutput(0, output);
	}
}

void ArgumentWord::Execute(Engine* e, WordInstance *instance)
{
	assert(instance != NULL);
	llvm::Value *output = JIT::GetSingleton().CreateInputArgument();
	instance->SetOutput(0, output);
}

void StringWord::Execute(Engine* e, WordInstance *instance)
{
	assert(instance != NULL);

	std::string string = e->GetLexer()->ReadUntil('"');

	// set string size
	llvm::Value *size = llvm::ConstantInt::get(llvm::APInt(32, string.size()));
	instance->SetOutput(0, size);

	// set string pointer
	llvm::Constant *string_constant = llvm::ConstantArray::get(string.c_str(), true);
	llvm::GlobalVariable *string_gv = new llvm::GlobalVariable(string_constant->getType(), true, llvm::GlobalValue::InternalLinkage, string_constant, "", JIT::GetSingleton().GetModule(), false);
	llvm::Value *ptr_to_int = JIT::GetSingleton().GetBuilder()->CreatePtrToInt(string_gv, llvm::Type::Int32Ty);
	instance->SetOutput(1, ptr_to_int);
}

