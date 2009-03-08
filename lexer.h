#pragma once

#include <iostream>

class EndOfStream : std::exception
{
	virtual const char *what() const throw()
	{
		return "End of stream";
	}
};

class Lexer
{
	std::istream &in;
public:
	Lexer(std::istream &_in);

	std::string NextWord();
	std::string NextToken();
	std::string ReadUntil(char u);
	std::string ReadLine();
private:
	void Skip(const std::string &close);
};

