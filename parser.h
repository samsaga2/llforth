#pragma once

class InferenceStack
{
public:
	class Counter
	{
	public:
		AST *ast;
		int index;
		Counter(AST *_ast)
			: ast(_ast), index(0) { }
	};

	std::vector<Counter*> stack;
	BodyAST args;

	InferenceStack() { }

	void Clear()
	{
		stack.clear();
		args.clear();
	}

	OutputIndexAST *Pop()
	{
		if(stack.size() == 0)
		{
			AST *arg = new ArgAST(args.size());
			args.push_back(arg);
			Push(arg);
		}

		assert(stack.size() != 0);
		Counter *counter = stack.back();
		OutputIndexAST *stack_index = new OutputIndexAST(counter->ast, counter->index);

		counter->index++;
		if(counter->index == counter->ast->OutputSize())
		{
			delete counter;
			stack.pop_back();
		}

		return stack_index;
	}

	BodyAST *Pop(int size)
	{
		BodyAST *body = new BodyAST();
		while(size > 0)
		{
			body->push_back(Pop());
			size--;
		}
	}

	void Push(AST *ast)
	{
		stack.push_back(new Counter(ast));
	}
};

class Parser
{
public:
	Lexer lexer;
	typedef std::list<FunctionAST *> Functions;
	Functions functions;
	InferenceStack istack;

	Parser(Lexer &_lexer) : lexer(_lexer) { }

	FunctionAST *FindFunction(const std::string &word)
	{
		Functions::reverse_iterator it = functions.rbegin();
		while(it != functions.rend())
		{
			FunctionAST *function = *it;
			if(function->Name() == word)
				return function;
			it++;
		}
		return NULL;
	}

	void AppendFunction()
	{
		std::string func_name = lexer.word;
		lexer.NextToken();

		FunctionAST *function = FindFunction(func_name);
		assert(function != NULL);

		AST *call = new CallAST(function, istack.Pop(function->InputSize()));
		istack.Push(call);
	}

	AST *AppendCore()
	{
		std::string word = lexer.word;
		lexer.NextToken();

		if(word == "dup")
		{
			OutputIndexAST *arg1 = istack.Pop();
			istack.Push(new DupAST(arg1));
		}
		else if(word == "*")
		{
			OutputIndexAST *arg2 = istack.Pop();
			OutputIndexAST *arg1 = istack.Pop();
			istack.Push(new MultAST(arg1, arg2));
		}
		else if(word == "+")
		{
			OutputIndexAST *arg2 = istack.Pop();
			OutputIndexAST *arg1 = istack.Pop();
			istack.Push(new AddAST(arg1, arg2));
		}
		else if(word == "swap")
		{
			OutputIndexAST *arg2 = istack.Pop();
			OutputIndexAST *arg1 = istack.Pop();
			istack.Push(new SwapAST(arg1, arg2));
		}
		else
		{
			std::string error("unknown token `");
			error += lexer.word;
			error += "'";
			throw error;
		}
	}

	void AppendInteger()
	{
		int integer = lexer.integer;
		lexer.NextToken();

		AST *ast = new IntegerAST(integer);
		istack.Push(ast);
	}

	void ParseWordExpr()
	{
		FunctionAST *function = FindFunction(lexer.word);
		if(function != NULL)
			AppendFunction();
		else
			switch(lexer.token)
			{
			default:
				throw std::string("end of file");
			case Lexer::tok_word:
				AppendCore();
				break;
			case Lexer::tok_integer:
				AppendInteger();
				break;
			}
	}

	void ParseBody(const std::string &end)
	{
		istack.Clear();
		do
		{
			if(lexer.word == end)
				break;

			ParseWordExpr();
		}
		while(true);
	}

	FunctionAST *ParseFunction()
	{
		assert(lexer.word == ":");
		lexer.NextToken();

		std::string func_name = lexer.word;
		lexer.NextToken();

		ParseBody(";");

		BodyAST *func_body = new BodyAST();
		while(istack.stack.size())
			func_body->push_front(istack.Pop());

		BodyAST *func_args = new BodyAST();
		for(BodyAST::iterator it = istack.args.begin(); it != istack.args.end(); it++)
			func_args->push_back(*it);

		return new FunctionAST(func_name, func_body, func_args);
	}

	void MainLoop()
	{
		while(true)
		{
			if(lexer.token == Lexer::tok_eof)
				break;
			else
			{
				FunctionAST *function = ParseFunction();
				functions.push_back(function);
				function->Print();
				lexer.NextToken();
			}
		}
	}

	void Compile()
	{
		Module module("llforth");

		for(Functions::iterator it = functions.begin(); it != functions.end(); it++)
			(*it)->Compile(&module);
	
		module.dump();
	}
};

