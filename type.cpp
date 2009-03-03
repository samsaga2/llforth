#include "type.h"
#include <iostream>
#include <llvm/DerivedTypes.h>

using namespace std;

void PrintType(TypeAST t)
{
	switch(t)
	{
		case TYPE_NULL: cout << " *"; break;
		case TYPE_INT32: cout << " i"; break;
		case TYPE_FLOAT: cout << " f"; break;
		case TYPE_STRING: cout << " s"; break;
		case TYPE_ANY: cout << " ?"; break;
	}
}

const Type *ConvertType(TypeAST t)
{
	switch(t)
	{
		case TYPE_NULL:
			throw string("not supported");

		case TYPE_INT32:
			return Type::Int32Ty;

		case TYPE_FLOAT:
			return Type::FloatTy;

		case TYPE_STRING:
			return PointerType::getUnqual(Type::Int8Ty);
	}
}

