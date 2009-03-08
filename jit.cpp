#include "jit.h"
#include <llvm/Analysis/Verifier.h>
#include <iostream>

JIT::JIT() : module("llforth")
{
	optimize = false;
	latest = NULL;
	builder = NULL;

	jit = llvm::ExecutionEngine::create(&module);

	module_provider = new llvm::ExistingModuleProvider(&module);
	fpm = new llvm::FunctionPassManager(module_provider);
	fpm->add(new llvm::TargetData(*jit->getTargetData()));
	fpm->add(llvm::createReassociatePass());
	fpm->add(llvm::createGVNPass());
	fpm->add(llvm::createCFGSimplificationPass());
	fpm->add(llvm::createPromoteMemoryToRegisterPass());
}

llvm::Value *JIT::CreateInputArgument()
{
	llvm::Argument *arg = new llvm::Argument(llvm::Type::Int32Ty);
	inp_args.push_back(arg);
	return arg;
}

llvm::Value *JIT::CreateOutputArgument()
{
	llvm::Argument *arg = new llvm::Argument(llvm::PointerType::getUnqual(llvm::Type::Int32Ty));
	out_args.push_back(arg);
	return arg;
}

void JIT::CreateWord()
{
	// create entry
	latest_entry = llvm::BasicBlock::Create("entry");
	builder = new llvm::IRBuilder<>(latest_entry);
}

void JIT::FinishWord(const std::string& word)
{
	size_t inputs = inp_args.size();
	size_t outputs = out_args.size();

	// argument types
	std::vector<const llvm::Type *> args(inputs + outputs);
	for(size_t i = 0; i < inputs; i++)
		args[i] = llvm::Type::Int32Ty;
	for(size_t i = inputs; i < inputs + outputs; i++)
		args[i] = llvm::PointerType::getUnqual(llvm::Type::Int32Ty);

	// create function
	llvm::FunctionType *ftype = llvm::FunctionType::get(llvm::Type::VoidTy, args, false);
	latest = llvm::Function::Create(ftype, llvm::Function::ExternalLinkage, word, &module);
	latest->getBasicBlockList().push_back(latest_entry);

	// fix input args
	std::list<llvm::Argument *>::iterator it1 = inp_args.begin();
	llvm::Function::arg_iterator it = latest->arg_begin();
	while(it1 != inp_args.end())
	{
		(*it1)->replaceAllUsesWith(&*it);
		delete *it1;
		it1++;
		it++;
	}
	inp_args.clear();

	// fix output args
	it1 = out_args.begin();
	while(it1 != out_args.end())
	{
		(*it1)->replaceAllUsesWith(&*it);
		delete *it1;
		it1++;
		it++;
	}
	out_args.clear();

	// optimize jit function
	llvm::verifyFunction(*latest);
	if(optimize)
		fpm->run(*latest);
}

