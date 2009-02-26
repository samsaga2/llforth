LLFORTH_SRCS = llforth.cpp

CC = g++
CFLAGS = -g
LDFLAGS = `llvm-config --cxxflags --ldflags --libs core`

all: llforth

llforth: llforth.cpp ast.h lexer.h parser.h
	$(CC) $(LLFORTH_SRCS) -o llforth $(LDFLAGS) $(CFLAGS)

clean:
	rm -f *.o llforth
	
