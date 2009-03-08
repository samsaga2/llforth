#include "jit.h"
#include <llvm/Analysis/Verifier.h>

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

void JIT::CreateWord(const std::string& word, size_t inputs, size_t outputs)
{
	// argument types
	std::vector<const llvm::Type *> args(inputs + outputs);
	for(size_t i = 0; i < inputs; i++)
		args[i] = llvm::Type::Int32Ty;
	for(size_t i = inputs; i < inputs + outputs; i++)
		args[i] = llvm::PointerType::getUnqual(llvm::Type::Int32Ty);

	// create function
	llvm::FunctionType *ftype = llvm::FunctionType::get(llvm::Type::VoidTy, args, false);
	latest = llvm::Function::Create(ftype, llvm::Function::ExternalLinkage, word, &module);

	// create entry
	latest_entry = llvm::BasicBlock::Create("entry", latest);
	builder = new llvm::IRBuilder<>(latest_entry);
}

llvm::Value *JIT::GetArgument(size_t index)
{
	llvm::Function::arg_iterator it = latest->arg_begin();
	for(size_t i = 0; i < index; i++,it++);
	return it;
}

void JIT::FinishWord()
{
	llvm::verifyFunction(*latest);
	if(optimize)
		fpm->run(*latest);
}

void JIT::AppendCall(llvm::Function *function)
{
	// TODO
}

