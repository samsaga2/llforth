#pragma once

class AST
{
public:
	AST() : compiled(false) { }
	Value *GetValue(int index, IRBuilder<> builder)
	{
		if(!compiled)
		{
			Compile(builder);
			compiled = true;
		}
		return values[index];
	}
	virtual int InputSize() = 0;
	virtual int OutputSize() = 0;
	virtual void Print() = 0;
	virtual ~AST() { }
protected:
	bool compiled;
	std::vector<Value *> values;
	void PushValue(Value *value)
	{
		values.push_back(value);
	}
	virtual void Compile(IRBuilder<> builder) = 0;
};

class IntegerAST : public AST
{
	int integer;
public:
	IntegerAST(int _integer)
		: integer(_integer) { }
	int InputSize() { return 0; }
	int OutputSize() { return 1; }
	void Print() { std::cout << integer; }
protected:
	void Compile(IRBuilder<> builder)
	{
		PushValue(ConstantInt::get(APInt(32, integer)));
	}
};

class BodyAST : public std::list<AST *>
{
public:
	int OutputSize()
	{
		int output_size = 0;
		for(BodyAST::iterator it = this->begin(); it != this->end(); it++)
			output_size += (*it)->OutputSize();
		return output_size;
	}
	void Print()
	{
		for(BodyAST::iterator it = this->begin(); it != this->end(); it++)
		{
			std::cout << " ";
			(*it)->Print();
		}
	}
};

class FunctionAST
{
	std::string name;
	BodyAST *body;
	BodyAST *args;
public:
	FunctionAST(const std::string &_name, BodyAST *_body, BodyAST *_args)
		: name(_name), body(_body), args(_args) { }
	const std::string Name() { return name; }
	int InputSize() { return args->OutputSize(); }
	int OutputSize() { return body->OutputSize(); }
	void Print()
	{
		std::cout << name << " ";
		for(int i = 0; i < args->OutputSize(); i++)
			std::cout << " n";
		std::cout << " -- ";
		for(int i = 0; i < body->OutputSize(); i++)
			std::cout << " n";

		for(BodyAST::iterator it = body->begin(); it != body->end(); it++)
		{
			std::cout << std::endl << "  ";
			(*it)->Print();
		}
		
		std::cout << std::endl << std::endl;
	}
	void Compile(Module *module)
	{
		std::vector<const Type *> args(InputSize() + OutputSize(), IntegerType::get(32));
		for(int i = InputSize(); i < args.size(); i++)
			args[i] = PointerType::get(IntegerType::get(32), 0);

		FunctionType *function_type = FunctionType::get(Type::VoidTy, args, false);
		Function *function = Function::Create(function_type, Function::ExternalLinkage, name, module);

		BasicBlock *entry = BasicBlock::Create("entry", function);
		IRBuilder<> builder(entry);

		// arg names
		Function::arg_iterator arg_it = function->arg_begin();
		for(int i = 0; i < InputSize(); i++)
		{
			std::ostringstream oss;
			oss << "inp" << i;
			arg_it->setName(oss.str());
			arg_it++;
		}
		for(int i = 0; i < OutputSize(); i++)
		{
			std::ostringstream oss;
			oss << "out" << i;
			arg_it->setName(oss.str());
			arg_it++;
		}

		// arg_it = output args
		arg_it = function->arg_begin();
		for(int i = 0; i < InputSize(); i++)
		{
			std::ostringstream oss;
			oss << "inp" << i;
			arg_it->setName(oss.str());
			arg_it++;
		}

		for(BodyAST::iterator it = body->begin(); it != body->end(); it++)
		{
			AST *ast = *it;
			for(int i = 0; i < ast->OutputSize(); i++)
				builder.CreateStore(ast->GetValue(i, builder), arg_it++);
		}

		builder.CreateRetVoid();
	}
};

class CallAST : public AST
{
	FunctionAST *function;
	BodyAST *args;
public:
	CallAST(FunctionAST *_function, BodyAST *_args)
		: function(_function), args(_args) { }
	int InputSize() { return function->InputSize(); }
	int OutputSize() { return function->OutputSize(); }
	void Print() { std::cout << function->Name(); }
protected:
	void Compile(IRBuilder<> builder) { std::cout << "call" << std::endl; }
};

class ArgAST : public AST
{
	int n;
public:
	ArgAST(int _n) : n(_n) { }
	int InputSize() { return 0; }
	int OutputSize() { return 1; }
	void Print() { std::cout << "arg" << n; }
protected:
	void Compile(IRBuilder<> builder)
	{
		BasicBlock *bb = builder.GetInsertBlock();
		Function *f = bb->getParent();
		Function::arg_iterator arg_it = f->arg_begin();
		for(int i = 0; i < n; i++,arg_it++);
		PushValue(arg_it);
	}
};

class OutputIndexAST : public AST
{
	AST *ast;
	int index;
public:
	OutputIndexAST(AST *_ast, int _index)
		: ast(_ast), index(_index) { }
	int InputSize() { return 0; }
	int OutputSize() { return 1; }
	void Print()
	{
		if(ast->OutputSize() == 1)
			ast->Print();
		else
		{
			std::cout << "[";
			ast->Print();
			std::cout << "]:" << index;
		}
	}
protected:
	void Compile(IRBuilder<> builder)
	{
		PushValue(ast->GetValue(index, builder));
	}
};

class DupAST : public AST
{
	OutputIndexAST *arg1;
public:
	DupAST(OutputIndexAST *_arg1) : arg1(_arg1) { }
	int InputSize() { return 1; }
	int OutputSize() { return 2; }
	void Print()
	{
		arg1->Print();
		std::cout << " ";
		arg1->Print();
	}
protected:
	void Compile(IRBuilder<> builder)
	{
		PushValue(arg1->GetValue(0, builder));
		PushValue(arg1->GetValue(0, builder));
	}
};

class MultAST : public AST
{
	OutputIndexAST *arg1;
	OutputIndexAST *arg2;
public:
	MultAST(OutputIndexAST *_arg1, OutputIndexAST *_arg2) : arg1(_arg1), arg2(_arg2) { }
	int InputSize() { return 2; }
	int OutputSize() { return 1; }
	void Print()
	{
		std::cout << "(";
		arg1->Print();
		std::cout << "*";
		arg2->Print();
		std::cout << ")";
	}
protected:
	void Compile(IRBuilder<> builder)
	{
		PushValue(builder.CreateMul(arg1->GetValue(0, builder), arg2->GetValue(0, builder)));
	}
};

class AddAST : public AST
{
	OutputIndexAST *arg1;
	OutputIndexAST *arg2;
public:
	AddAST(OutputIndexAST *_arg1, OutputIndexAST *_arg2) : arg1(_arg1), arg2(_arg2) { }
	int InputSize() { return 2; }
	int OutputSize() { return 1; }
	void Print()
	{
		std::cout << "(";
		arg1->Print();
		std::cout << "+";
		arg2->Print();
		std::cout << ")";
	}
protected:
	void Compile(IRBuilder<> builder)
	{
		PushValue(builder.CreateAdd(arg1->GetValue(0, builder), arg2->GetValue(0, builder)));
	}
};

class SwapAST : public AST
{
	OutputIndexAST *arg1;
	OutputIndexAST *arg2;
public:
	SwapAST(OutputIndexAST *_arg1, OutputIndexAST *_arg2) : arg1(_arg1), arg2(_arg2) { }
	int InputSize() { return 2; }
	int OutputSize() { return 2; }
	void Print()
	{
		arg2->Print();
		std::cout << " ";
		arg1->Print();
	}
protected:
	void Compile(IRBuilder<> builder)
	{
		PushValue(arg2->GetValue(0, builder));
		PushValue(arg1->GetValue(0, builder));
	}
};

