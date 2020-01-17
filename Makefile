# Personal Makefile Template.
CXX = g++
CXX_FLAGS = -std=c++17 -Wconversion -Wall -Werror -Wextra -pedantic
OPT = -O3 -DNDEBUG
DEBUG = -g3 -DDEBUG

EXECUTABLE = benchmark
LINKED_FILES = trie

# Expand the linked file names into lists of .cpp and .o files.
LINKED_CPP = $(foreach file, $(LINKED_FILES), $(file).cpp)
LINKED_O = $(foreach file, $(LINKED_FILES), $(file).o)

# Build optimized executable - ensure clean slate.
release : clean $(EXECUTABLE).cpp $(LINKED_CPP)
	$(CXX) $(CXX_FLAGS) $(OPT) -c $(EXECUTABLE).cpp $(LINKED_CPP)
	$(CXX) $(CXX_FLAGS) $(OPT) $(EXECUTABLE).o $(LINKED_O) -o $(EXECUTABLE)

# Build with debug features - ensure clean slate.
debug : clean $(EXECUTABLE).cpp $(LINKED_CPP)
	$(CXX) $(CXX_FLAGS) $(DEBUG) -c $(EXECUTABLE).cpp $(LINKED_CPP)
	$(CXX) $(CXX_FLAGS) $(DEBUG) $(EXECUTABLE).o $(LINKED_O) -o $(EXECUTABLE)_debug

# Build with no compiler optimization - ensure clean slate.
noopt : clean $(EXECUTABLE).cpp $(LINKED_CPP)
	$(CXX) $(CXX_FLAGS) -c $(EXECUTABLE).cpp $(LINKED_CPP)
	$(CXX) $(CXX_FLAGS) $(EXECUTABLE).o $(LINKED_O) -o $(EXECUTABLE)

# Remove executable binary and generated objected files.
.PHONY : clean
clean : 
	rm -f $(EXECUTABLE) $(EXECUTABLE)_debug $(EXECUTABLE).o $(LINKED_O)