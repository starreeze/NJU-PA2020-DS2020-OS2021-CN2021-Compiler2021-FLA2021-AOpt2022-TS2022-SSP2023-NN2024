CXX = g++ -c
CXXFLAGS = -std=c++11 -O2 -DNDEBUG
CXXFLAGS_DEBUG = -std=c++11 -O0 -g -DDEBUG
LD = g++
LDFLAGS = 
TARGET = turing

turing: compile
	$(LD) $(LDFALGS) $(shell find ./ -name "*.o") -o $(TARGET)

debug: compile_debug
	$(LD) $(LDFALGS) $(shell find ./ -name "*.o") -o $(TARGET)

compile:
	$(CXX) $(CXXFLAGS) $(shell find ./ -name "*.cpp")

compile_debug:
	$(CXX) $(CXXFLAGS_DEBUG) $(shell find ./ -name "*.cpp")

clean:
	rm -f *.o turing
