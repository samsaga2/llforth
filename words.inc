void word_dots()
{
	Engine &e = Engine::GetSingleton();
	for(std::list<int>::reverse_iterator it = e.runtime_stack.rbegin(); it != e.runtime_stack.rend(); it++)
		std::cout << "  " << *it;
	std::cout << std::endl;
}

void word_see()
{
	std::string word = Engine::GetSingleton().GetLexer()->NextWord();
	llvm::Function *function = JIT::GetSingleton().GetModule()->getFunction(word);
	if(function != NULL)
		function->dump();
}

void word_extern()
{
	Engine &e = Engine::GetSingleton();
	std::string function_name = e.GetLexer()->NextToken();

	// assert (
	assert(e.GetLexer()->NextWord() == "(");

	bool inputs = true;
	size_t inputs_size = 0;
	size_t outputs_size = 0;
	while(true)
	{
		std::string token = e.GetLexer()->NextWord();
		if(token == ")")
			break;
		else if(token == "i")
		{
			if(inputs)
				inputs_size++;
			else
				outputs_size++;
		}
		else if(token == "--")
		{
			assert(inputs);
			inputs = false;
		}
		else
			assert(false && "Unknown type");
	}
	assert(!inputs && "Missing `--' parsing extern");

	e.CreateExternWord(function_name, inputs_size, outputs_size);

	if(e.GetVerbose())
		JIT::GetSingleton().GetLatest()->dump();
}

void word_immediate()
{
	Engine::GetSingleton().GetLatest()->SetImmediate(true);
}

void word_colon()
{
	Engine &e = Engine::GetSingleton();
	std::string function_name = e.GetLexer()->NextToken();
	
	// create function
	e.CreateWord();

	// read body
	while(true)
	{
		std::string token = e.GetLexer()->NextToken();
		if(token == ";")
			break;

		// find word
		Word *word = e.FindWord(token);
		if(word == NULL)
		{
			// integer?
			std::istringstream is(token);
			int number;
			if(is >> number)
				word = new LiteralWord(number);
		}
		else if(word->IsImmediate())
		{
			// inline
			word->Execute(NULL);
			continue;
		}

		assert(word != NULL);

		// create word instance
		WordInstance *instance = new WordInstance(word);
		instance->Compile();

		e.Push(instance);
	}

	// print word info
	if(e.GetVerbose())
		std::cerr << "WORD: " << function_name << " ins:" << e.compiler_args.size() << " outs:" << e.compiler_stack.size() << std::endl;

	// setup outputs
	llvm::Function *latest = JIT::GetSingleton().GetLatest();
	for(size_t i = 0; !e.compiler_stack.empty(); i++)
	{
		llvm::Value *input = e.compiler_stack.back()->GetOutput();
		llvm::Value *output = JIT::GetSingleton().CreateOutputArgument();

		JIT::GetSingleton().GetBuilder()->CreateStore(input, output);
		e.compiler_stack.pop_back();
	}

	JIT::GetSingleton().GetBuilder()->CreateRetVoid();
	e.FinishWord(function_name);

	if(e.GetVerbose())
		JIT::GetSingleton().GetLatest()->dump();
}

