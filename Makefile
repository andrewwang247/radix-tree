# Executable name

RELEASE_EXE := perf_test
DEBUG_EXE := unit_test

# Compiler flags

CXX := clang++
CPPFLAGS := -MMD
CXXFLAGS := -std=c++20 -Wall -Werror -Wextra -Wconversion -pedantic -Wfloat-equal -Wshadow -Wdouble-promotion -Wundef
OPT := -O3 -DNDEBUG
DEBUG := -g3 -DDEBUG

# Directory structure

SRC_DIR := ./src
BUILD_DIR := ./build
RELEASE_DIR := $(BUILD_DIR)/release
DEBUG_DIR := $(BUILD_DIR)/debug

# Gather all expected .cpp files for each executable and expected .o

RELEASE_CPP := $(filter-out $(SRC_DIR)/$(DEBUG_EXE).cpp, $(wildcard $(SRC_DIR)/*.cpp))
DEBUG_CPP := $(filter-out $(SRC_DIR)/$(RELEASE_EXE).cpp, $(wildcard $(SRC_DIR)/*.cpp))

# Map .cpp files to their respective .o files

RELEASE_OBJS := $(RELEASE_CPP:$(SRC_DIR)/%.cpp=$(RELEASE_DIR)/%.o)
DEBUG_OBJS := $(DEBUG_CPP:$(SRC_DIR)/%.cpp=$(DEBUG_DIR)/%.o)

# Build release or debug executables

.PHONY: release
release: $(RELEASE_DIR)/$(RELEASE_EXE)

.PHONY: debug
debug: $(DEBUG_DIR)/$(DEBUG_EXE)

# Run application or tests

.PHONY: run
run: release
	$(RELEASE_DIR)/$(RELEASE_EXE)

.PHONY: test
test: debug
	$(DEBUG_DIR)/$(DEBUG_EXE)

# Link .o object files

$(RELEASE_DIR)/$(RELEASE_EXE): $(RELEASE_OBJS)
	$(CXX) $(CXXFLAGS) $(OPT) $^ -o $@

$(DEBUG_DIR)/$(DEBUG_EXE): $(DEBUG_OBJS)
	$(CXX) $(CXXFLAGS) $(DEBUG) $^ -o $@

# Compile .cpp sources

$(RELEASE_DIR)/%.o: $(SRC_DIR)/%.cpp | $(RELEASE_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(OPT) -c $< -o $@

$(DEBUG_DIR)/%.o: $(SRC_DIR)/%.cpp | $(DEBUG_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEBUG) -c $< -o $@

# Create build directories

$(RELEASE_DIR) $(DEBUG_DIR):
	mkdir -p $@

# Delete build directory

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

# Include .d dependencies

RELEASE_DEPS := $(RELEASE_CPP:$(SRC_DIR)/%.cpp=$(RELEASE_DIR)/%.d)
DEBUG_DEPS := $(DEBUG_CPP:$(SRC_DIR)/%.cpp=$(DEBUG_DIR)/%.d)

-include $(RELEASE_DEPS) $(DEBUG_DEPS)
