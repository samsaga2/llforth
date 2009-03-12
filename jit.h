#pragma once

#include <llvm/Module.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Support/IRBuilder.h>
#include <llvm/ModuleProvider.h>
#include <llvm/PassManager.h>
#include <llvm/CodeGen/Passes.h>
#include <llvm/LinkAllPasses.h>
#include <llvm/Target/TargetData.h>
#include <list>
#include <map>
#include "words.h"

class JIT
{
	bool optimize;

	llvm::Module module;
	llvm::ExecutionEngine *jit;
	llvm::ExistingModuleProvider *module_provider;
	llvm::FunctionPassManager *fpm;
	llvm::Function *latest;
	llvm::BasicBlock *latest_entry;
	llvm::IRBuilder<> *builder;
	std::list<llvm::Argument *> inp_args;
	std::list<llvm::Argument *> out_args;
	std::map<std::string, void *> extern_symbols;

	JIT();
public:
	static JIT &GetSingleton();

	void SetOptimize(bool optimize) { this->optimize = optimize; }

	llvm::Module *GetModule() { return &module; }
	llvm::IRBuilder<> *GetBuilder() { return builder; }
	llvm::Function *GetLatest() { return latest; }
	llvm::ExecutionEngine *GetExecutionEngine() { return jit; }

	void CreateExternWord(const std::string &word, size_t inputs, size_t outputs);
	void CreateWord();
	void FinishWord(const std::string& word);

	llvm::Value *CreateInputArgument();
	llvm::Value *CreateOutputArgument();
	size_t GetInputSize() { return inp_args.size(); }
	size_t GetOutputSize() { return out_args.size(); }

	void AddInternalSymbol(const std::string &name, void *address) { extern_symbols[name] = address; }
	void *FindSymbol(const std::string &str);
};

