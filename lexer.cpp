#include "lexer.h"
#include <sstream>
#include <assert.h>

Lexer::Lexer(std::istream &_in) : in(_in)
{
}

void Lexer::NextWord()
{
	if(!(in >> word))
	{
		token = tok_eof;
		return;
	}

	token = tok_word;
}

void Lexer::NextToken()
{
	NextWord();
	if(token == tok_eof)
		return;

	if(word == "\\")
	{
		ReadLine();
		NextToken();
	}
	else if(word == "(")
		Skip("(", ")");
	else if(word == "(*")
		Skip("(*", "*)");
	else
	{
		std::istringstream iss(word);
		if(iss >> integer)
			token = tok_integer;
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

void Lexer::ReadLine()
{
	if(!std::getline(in, word))
		token = tok_eof;
	else
		token = tok_word;
}

void Lexer::Skip(const std::string &open, const std::string &close)
{
	assert(word == open);
	while(true)
	{
		NextToken();
		if(token == tok_eof)
			break;
		else if(word == close)
		{
			NextToken();
			break;
		}
	}
}

