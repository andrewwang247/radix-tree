# Executable name

EXE := benchmark

# Compiler flags

CXX := g++
CPPFLAGS := -MMD
CXXFLAGS := -std=c++17 -Wall -Werror -Wextra -Wconversion -pedantic -Wfloat-equal -Wshadow -Wdouble-promotion -Wundef
OPT := -O3 -DNDEBUG
DEBUG := -g3 -DDEBUG

# Directory structure

SRC_DIR := ./src
BUILD_DIR := ./build
RELEASE_DIR := $(BUILD_DIR)/release
DEBUG_DIR := $(BUILD_DIR)/debug

# Gather all .cpp files and expected .o and .d files

CPP := $(wildcard $(SRC_DIR)/*.cpp)
RELEASE_OBJS := $(CPP:$(SRC_DIR)/%.cpp=$(RELEASE_DIR)/%.o)
DEBUG_OBJS := $(CPP:$(SRC_DIR)/%.cpp=$(DEBUG_DIR)/%.o)

# Build release or debug executables

.PHONY: release
release: $(RELEASE_DIR)/$(EXE)

.PHONY: debug
debug: $(DEBUG_DIR)/$(EXE)

# Link object files

$(RELEASE_DIR)/$(EXE): $(RELEASE_OBJS)
	$(CXX) $(CXXFLAGS) $(OPT) $^ -o $@ $(LDFLAGS)

$(DEBUG_DIR)/$(EXE): $(DEBUG_OBJS)
	$(CXX) $(CXXFLAGS) $(DEBUG) $^ -o $@ $(LDFLAGS)

# Compile CPP sources

$(RELEASE_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(RELEASE_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(OPT) -c $< -o $@

$(DEBUG_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(DEBUG_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEBUG) -c $< -o $@

# Remove executable and object files

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

# Include dependencies

RELEASE_DEPS := $(CPP:$(SRC_DIR)/%.cpp=$(RELEASE_DIR)/%.d)
DEBUG_DEPS := $(CPP:$(SRC_DIR)/%.cpp=$(DEBUG_DIR)/%.d)

-include $(RELEASE_DEPS) $(DEBUG_DEPS)
