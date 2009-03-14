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

void word_inline()
{
	Engine::GetSingleton().GetLatest()->SetInline(true);
}

