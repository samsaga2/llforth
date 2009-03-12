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

