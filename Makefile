OBJECTS := $(patsubst %.cpp,%.o,$(wildcard *.cpp))

CC = g++
CFLAGS = -g -Wno-deprecated `llvm-config --cxxflags`
LDFLAGS = `llvm-config --ldflags --libs`

.SUFFIXES:	.o .cpp

.cpp.o:
	$(CC) -c $< $(CFLAGS)

all: llforth

llforth: $(OBJECTS)
	$(CC) $(OBJECTS) -o llforth $(LDFLAGS) $(CFLAGS)

test:
	./llforth -v -O -i test.llfs -o test.obj
	llvm-ld test.obj --native 

clean:
	rm -f *.o llforth test.obj a.out a.out.bc

