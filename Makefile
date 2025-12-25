# Personal Makefile Template.
CXX = g++ -std=c++17
CXX_FLAGS = -Wall -Werror -Wextra -Wconversion -pedantic -Wfloat-equal -Wshadow -Wdouble-promotion -Wundef
OPT = -O3 -DNDEBUG
DEBUG = -g3 -DDEBUG

EXECUTABLE = benchmark
LINKED = trie

# Build optimized executable - ensure clean slate.
release : $(EXECUTABLE).cpp $(LINKED).cpp
	$(CXX) $(CXX_FLAGS) $(OPT) -c $(EXECUTABLE).cpp $(LINKED).cpp
	$(CXX) $(CXX_FLAGS) $(OPT) $(EXECUTABLE).o $(LINKED).o -o $(EXECUTABLE)

# Build with debug features - ensure clean slate.
debug : $(EXECUTABLE).cpp $(LINKED).cpp
	$(CXX) $(CXX_FLAGS) $(DEBUG) -c $(EXECUTABLE).cpp $(LINKED).cpp
	$(CXX) $(CXX_FLAGS) $(DEBUG) $(EXECUTABLE).o $(LINKED).o -o $(EXECUTABLE)

# Remove executable binary and generated objected files.
.PHONY : clean
clean : 
	rm -f $(EXECUTABLE) $(EXECUTABLE).o $(LINKED).o
