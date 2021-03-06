#include "jit.h"
#include <llvm/Analysis/Verifier.h>
#include <llvm/CallingConv.h>
#include <iostream>
#include <dlfcn.h>

static void *findSymbol(const std::string &str)
{
	JIT::GetSingleton().FindSymbol(str);
}

JIT::JIT() : module("llforth")
{
	optimize = false;
	latest = NULL;
	builder = NULL;

	jit = llvm::ExecutionEngine::create(&module);
	jit->InstallLazyFunctionCreator(findSymbol);

	module_provider = new llvm::ExistingModuleProvider(&module);
	fpm = new llvm::FunctionPassManager(module_provider);
	fpm->add(new llvm::TargetData(*jit->getTargetData()));
	fpm->add(llvm::createReassociatePass());
	fpm->add(llvm::createGVNPass());
	fpm->add(llvm::createCFGSimplificationPass());
	fpm->add(llvm::createPromoteMemoryToRegisterPass());
}

JIT &JIT::GetSingleton()
{
	static JIT jit;
	return jit;
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

void JIT::CreateExternWord(const std::string &word, size_t inputs, size_t outputs)
{
	std::vector<const llvm::Type *> args;

	// input arguments
	for(size_t i = 0; i < inputs; i++)
		args.push_back(llvm::Type::Int32Ty);

	// output arguments
	const llvm::Type *ret_type = llvm::Type::VoidTy;
	if(outputs == 1)
		ret_type = llvm::Type::Int32Ty;
	else if(outputs > 19)
		for(size_t i = 0; i < outputs; i++)
			args.push_back(llvm::PointerType::getUnqual(llvm::Type::Int32Ty));
	
	// create function
	llvm::FunctionType *ftype = llvm::FunctionType::get(ret_type, args, false);
	latest = llvm::Function::Create(ftype, llvm::Function::ExternalLinkage, word, &module);
}

void JIT::CreateWord()
{
	// create entry
	latest_entry = llvm::BasicBlock::Create("entry");
	builder = new llvm::IRBuilder<>(latest_entry);
}

void JIT::FinishWord(const std::string &word)
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
	if(optimize)
		latest->setCallingConv(llvm::CallingConv::Fast);
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

void *JIT::FindSymbol(const std::string &str)
{
	if(extern_symbols.find(str) == extern_symbols.end())
		return dlsym(RTLD_DEFAULT, str.c_str());
	else
		return extern_symbols[str];
}

