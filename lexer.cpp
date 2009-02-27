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

