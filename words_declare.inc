IWORD(".s", word_dots, 0, 0);
IWORD("see", word_see, 0, 0);
IWORD("extern", word_extern, 0, 0);
IWORD("inline", word_inline, 0, 0); INLINE();
IWORD(":", word_colon, 0, 0);
WORD(StringWord);

BWORD("+");
	ARG(0);
	ARG(1);
	OUT(0, BUILDER->CreateAdd(arg0, arg1));
EWORD();

BWORD("-");
	ARG(0);
	ARG(1);
	OUT(0, BUILDER->CreateSub(arg0, arg1));
EWORD();

BWORD("*");
	ARG(0);
	ARG(1);
	OUT(0, BUILDER->CreateMul(arg0, arg1));
EWORD();

BWORD("/");
	ARG(0);
	ARG(1);
	OUT(0, BUILDER->CreateSDiv(arg0, arg1));
EWORD();

BWORD("drop");
	ARG(0);
EWORD();

BWORD("dup");
	ARG(0);
	OUT(0, arg0);
	OUT(1, arg0);
EWORD();

BWORD("over");
	ARG(0);
	ARG(1);
	OUT(0, arg1);
	OUT(1, arg0);
	OUT(2, arg1);
EWORD();

BWORD("rot");
	ARG(0);
	ARG(1);
	ARG(2);
	OUT(0, arg1);
	OUT(1, arg0);
	OUT(2, arg2);
EWORD();

BWORD("swap");
	ARG(0);
	ARG(1);
	OUT(0, arg0);
	OUT(1, arg1);
EWORD();

BWORD("nip");
	ARG(0);
	ARG(1);
	OUT(0, arg0);
EWORD();

