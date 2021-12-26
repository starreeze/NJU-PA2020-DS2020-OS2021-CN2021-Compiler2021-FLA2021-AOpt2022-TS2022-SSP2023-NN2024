CC = gcc -c
CFLAGS = -std=c99 -O2 -DOPTIMIZE -DNDEBUG
CFLAGS_DEBUG = -std=c99 -O0 -g -DOPTIMIZE
CXX = g++ -c
CXXFLAGS = -std=c++11 -O2 -DNDEBUG
CXXFLAGS_DEBUG = -std=c++11 -O0 -g
LD = g++
LDFLAGS = -lfl
TARGET = parser


parser: compile
	$(LD) $(LDFALGS) $(shell find ./ -name "*.o") -o $(TARGET)

debug: compile_debug
	$(LD) $(LDFALGS) $(shell find ./ -name "*.o") -o $(TARGET)

compile: syntax
	$(CC) $(CFLAGS) $(shell find ./ -name "*.c")
	$(CXX) $(CXXFLAGS) $(shell find ./ -name "*.cpp")

compile_debug: syntax
	$(CC) $(CFLAGS_DEBUG) $(shell find ./ -name "*.c")
	$(CXX) $(CXXFLAGS_DEBUG) $(shell find ./ -name "*.cpp")


# lab1
parser1:
	bison -d syntax.y
	flex lexical.l
	gcc -O2 syntax.tab.c lex.yy.c main.c gtree.c -lfl -o parser

debug1:
	bison -d -v --debug syntax.y
	flex lexical.l
	gcc -Werror -O0 -g syntax.tab.c lex.yy.c main.c gtree.c -lfl -o parser
	

# common
syntax:
	bison -d syntax.y
	flex lexical.l

clean:
	rm -f lex.yy.c parser syntax.tab.* syntax.output *.o
