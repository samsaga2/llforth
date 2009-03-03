#pragma once

#include <istream>

class Lexer
{
public:
	enum Token
	{
		tok_eof = -1,
		tok_word = -2,
		tok_integer = -3,
		tok_float = -4
	};

	std::istream &in;
	int token;
	std::string word;
	int number_integer;
	float number_float;

	Lexer(std::istream &_in);

	void NextWord();
	void NextToken();
	void ReadUntil(char u);
	void ReadLine();
private:
	void Skip(const std::string &open, const std::string &close);
};

