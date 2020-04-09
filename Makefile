# Personal Makefile Template.
CXX = g++ -std=c++17
CXX_FLAGS = -Wall -Werror -Wextra -Wconversion -pedantic -Wfloat-equal -Wduplicated-branches -Wduplicated-cond -Wshadow -Wdouble-promotion -Wundef
OPT = -O3 -DNDEBUG
DEBUG = -g3 -DDEBUG

EXECUTABLE = benchmark
LINKED_FILES = trie

# Expand the linked file names into lists of .cpp and .o files.
LINKED_CPP = $(foreach file, $(LINKED_FILES), $(file).cpp)
LINKED_O = $(foreach file, $(LINKED_FILES), $(file).o)

# Build optimized executable - ensure clean slate.
release : $(EXECUTABLE).cpp $(LINKED_CPP)
	$(CXX) $(CXX_FLAGS) $(OPT) -c $(EXECUTABLE).cpp $(LINKED_CPP)
	$(CXX) $(CXX_FLAGS) $(OPT) $(EXECUTABLE).o $(LINKED_O) -o $(EXECUTABLE)

# Build with debug features - ensure clean slate.
debug : $(EXECUTABLE).cpp $(LINKED_CPP)
	$(CXX) $(CXX_FLAGS) $(DEBUG) -c $(EXECUTABLE).cpp $(LINKED_CPP)
	$(CXX) $(CXX_FLAGS) $(DEBUG) $(EXECUTABLE).o $(LINKED_O) -o $(EXECUTABLE)

# Remove executable binary and generated objected files.
.PHONY : clean
clean : 
	rm -f $(EXECUTABLE) $(EXECUTABLE).o $(LINKED_O)
