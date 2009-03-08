#pragma once

#include <llvm/Module.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Support/IRBuilder.h>
#include <llvm/ModuleProvider.h>
#include <llvm/PassManager.h>
#include <llvm/CodeGen/Passes.h>
#include <llvm/LinkAllPasses.h>
#include <llvm/Target/TargetData.h>

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
public:
	JIT();

	void SetOptimize(bool optimize) { this->optimize = optimize; }

	llvm::Module *GetModule() { return &module; }
	llvm::Value *GetArgument(size_t index);
	llvm::IRBuilder<> *GetBuilder() { return builder; }
	llvm::Function *GetLatest() { return latest; }
	llvm::ExecutionEngine *GetExecutionEngine() { return jit; }

	void CreateWord(const std::string& word, size_t inputs, size_t outputs);
	void FinishWord();
	void AppendCall(llvm::Function *function);
};

