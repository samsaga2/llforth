#include "type.h"
#include <iostream>
#include <llvm/DerivedTypes.h>

using namespace std;

void PrintType(TypeAST t)
{
	switch(t)
	{
		case TYPE_INT32: cout << " i"; break;
		case TYPE_FLOAT: cout << " f"; break;
	}
}

const Type *ConvertType(TypeAST t)
{
	switch(t)
	{
		case TYPE_INT32:
			return Type::Int32Ty;

		case TYPE_FLOAT:
			return Type::FloatTy;
	}
}

TypeAST ConvertTypeAST(const std::string &type)
{
	if(type == "i")
		return TYPE_INT32;
	else if(type == "f")
		return TYPE_FLOAT;
	else
	{
		std::string error = "unknown type `";
		error += type;
		error += "'";
		throw error;
	}
}

