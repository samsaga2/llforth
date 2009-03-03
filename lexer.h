#pragma once

#include <istream>

class EndOfStream : std::exception
{
	virtual const char *what() const throw()
	{
		return "End of stream";
	}
};

class Lexer
{
public:
	enum Token
	{
		tok_word,
		tok_integer,
		tok_float
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

