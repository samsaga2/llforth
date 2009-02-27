#include "lexer.h"
#include <sstream>

Lexer::Lexer(std::istream &_in) : in(_in)
{
}

void Lexer::NextToken()
{
	if(in.eof())
		token = tok_eof;
	else
	{
		in >> word; 

		std::istringstream iss(word);
		if(iss >> integer)
			token = tok_integer;
		else
			token = tok_word;
	}
}

void Lexer::ReadUntil(char u)
{
	token = tok_word;
	if(word[word.size() - 1] == u)
	{
		word.resize(word.size() - 1);
		return;
	}

	do
	{
		char c = in.get();
		if(c == u)
			return;
		word += c;
	}
	while(!in.eof());

	token = tok_eof;
}

