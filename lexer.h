#pragma once

#include <istream>

class Lexer
{
public:
	enum Token
	{
		tok_eof = -1,
		tok_word = -2,
		tok_integer = -3
	};

	std::istream &in;
	int token;
	std::string word;
	int integer;

	Lexer(std::istream &_in);

	void NextToken();
	void ReadUntil(char u);
	void ReadLine();
};

