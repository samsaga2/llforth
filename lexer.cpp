#include "lexer.h"
#include <sstream>

Lexer::Lexer(std::istream &_in) : in(_in)
{
}

void Lexer::NextToken()
{
	if(!(in >> word))
	{
		token = tok_eof;
		return;
	}

	std::istringstream iss(word);
	if(iss >> integer)
		token = tok_integer;
	else
		token = tok_word;
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

void Lexer::ReadLine()
{
	if(!std::getline(in, word))
		token = tok_eof;
	else
		token = tok_word;
}

