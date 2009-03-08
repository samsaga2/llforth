#include "lexer.h"
#include <sstream>
#include <assert.h>

using namespace std;

Lexer::Lexer(istream &_in) : in(_in)
{
}

void Lexer::NextWord()
{
	if(!(in >> word))
		throw EndOfStream();

	token = tok_word;
}

void Lexer::NextToken()
{
	NextWord();

	if(word == "\\")
	{
		ReadLine();
		NextToken();
	}
	else if(word == "(")
		Skip("(", ")");
	else if(word == "(*")
		Skip("(*", "*)");
	else if(word.find('.') != string::npos)
	{
		// float?
		istringstream iss(word);
		if(iss >> number_float)
			token = tok_float;
	}
	else
	{
		// integer?
		istringstream iss(word);
		if(iss >> number_integer)
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
		if(c == u) break;
		word += c;
	}
	while(true);
}

void Lexer::ReadLine()
{
	if(!getline(in, word))
		throw EndOfStream();
	else
		token = tok_word;
}

void Lexer::Skip(const string &open, const string &close)
{
	assert(word == open);
	while(true)
	{
		NextToken();
		if(word == close)
		{
			NextToken();
			break;
		}
	}
}

