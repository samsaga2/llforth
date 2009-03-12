#include "lexer.h"

Lexer::Lexer(std::istream &_in) : in(_in)
{
}

std::string Lexer::NextWord()
{
	std::string word;
	if(!(in >> word))
		throw EndOfStream();

	return word;
}

std::string Lexer::NextToken()
{
	std::string word = NextWord();

	if(word == "\\")
	{
		ReadLine();
		return NextToken();
	}
	else if(word == "(")
	{
		Skip(")");
		return NextToken();
	}
	else if(word == "(*")
	{
		Skip("*)");
		return NextToken();
	}
	else
		return word;
}

std::string Lexer::ReadUntil(char u)
{
	std::string word = "";

	// skip whitespace
	char c = in.get();
	while(c <= ' ')
		c = in.get();

	// read until u
	while(c != u)
	{
		word += c;
		c = in.get();
	}

	return word;
}

std::string Lexer::ReadLine()
{
	std::string line;
	if(!std::getline(in, line))
		throw EndOfStream();
	else
		return line;
}

void Lexer::Skip(const std::string &close)
{
	std::string word;
	do
	{
		word = NextToken();
	}
	while(word != close);
}

