# Executable name

EXE := benchmark

# Compiler flags

CXX := clang++
CPPFLAGS := -MMD
CXXFLAGS := -std=c++20 -Wall -Werror -Wextra -Wconversion -pedantic -Wfloat-equal -Wshadow -Wdouble-promotion -Wundef
OPT := -O3 -DNDEBUG
DEBUG := -g3 -DDEBUG
EXTRA :=

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

# Run application or tests

.PHONY: run
run: release
	$(RELEASE_DIR)/$(EXE)

.PHONY: test
test: debug
	$(DEBUG_DIR)/$(EXE)

# Link .o object files

$(RELEASE_DIR)/$(EXE): $(RELEASE_OBJS)
	$(CXX) $(CXXFLAGS) $(OPT) $(EXTRA) $^ -o $@

$(DEBUG_DIR)/$(EXE): $(DEBUG_OBJS)
	$(CXX) $(CXXFLAGS) $(DEBUG) $(EXTRA) $^ -o $@

# Compile .cpp sources

$(RELEASE_DIR)/%.o: $(SRC_DIR)/%.cpp | $(RELEASE_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(OPT) $(EXTRA) -c $< -o $@

$(DEBUG_DIR)/%.o: $(SRC_DIR)/%.cpp | $(DEBUG_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEBUG) $(EXTRA) -c $< -o $@

# Create build directories

$(RELEASE_DIR) $(DEBUG_DIR):
	mkdir -p $@

# Delete build directory

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

# Include .d dependencies

RELEASE_DEPS := $(CPP:$(SRC_DIR)/%.cpp=$(RELEASE_DIR)/%.d)
DEBUG_DEPS := $(CPP:$(SRC_DIR)/%.cpp=$(DEBUG_DIR)/%.d)

-include $(RELEASE_DEPS) $(DEBUG_DEPS)
