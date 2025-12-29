# Compiler and flags.
CXX := g++ -std=c++17 
FLAGS := -Wall -Werror -Wextra -Wconversion -pedantic -Wfloat-equal -Wshadow -Wdouble-promotion -Wundef
OPT := -O3 -DNDEBUG
DEBUG := -g3 -DDEBUG

# Executable name
EXE := benchmark

# Expand the linked file names into lists of .cpp and .o files.
LINKED_CPP := $(filter-out $(EXE).cpp, $(wildcard *.cpp))
LINKED_O := $(LINKED_CPP:.cpp=.o)

# Build optimized executable.
release : $(EXE).cpp $(LINKED_CPP)
	$(CXX) $(FLAGS) $(OPT) -c $(EXE).cpp $(LINKED_CPP)
	$(CXX) $(FLAGS) $(OPT) $(EXE).o $(LINKED_O) -o $(EXE)

# Build with debug features.
debug : $(EXE).cpp $(LINKED_CPP)
	$(CXX) $(FLAGS) $(DEBUG) -c $(EXE).cpp $(LINKED_CPP)
	$(CXX) $(FLAGS) $(DEBUG) $(EXE).o $(LINKED_O) -o $(EXE)

# Remove executable binary and generated objected files.
.PHONY : clean
clean : 
	rm -f $(EXE) $(EXE).o $(LINKED_O)
