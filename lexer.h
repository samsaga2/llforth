#pragma once

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

	Lexer(std::istream &_in) : in(_in) { }

	void NextToken()
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
};

