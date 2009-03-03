OBJECTS := $(patsubst %.cpp,%.o,$(wildcard *.cpp))

CC = g++
#CFLAGS = -g -Wno-deprecated -I/usr/local/include  -D_DEBUG  -D_GNU_SOURCE -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -fomit-frame-pointer -fPIC #`llvm-config --cxxflags`
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

