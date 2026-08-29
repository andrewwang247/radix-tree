#!/bin/bash

# Copyright 2026. Andrew Wang.
# Run static checks on all C++ code.

set -uo pipefail

# List of all C++ files.
hfiles="src/*.h"
cppfiles="src/*.cpp"

printf "Running clang-format...\n"
clang-format -i -style=file $hfiles $cppfiles

printf "Running cppcheck...\n"
cppcheck --language=c++ --std=c++23 --quiet \
    --check-level=exhaustive \
    --enable=all \
    --template=gcc \
    --suppress=checkersReport \
    --suppress=missingIncludeSystem \
    --suppress=unusedStructMember \
    $hfiles $cppfiles

printf "Running cpplint...\n"
cpplint --filter=-build/include_subdir --quiet $hfiles $cppfiles

if [[ ! -f "compile_commands.json" ]]; then
    printf "Generating compile commands...\n"
    bear -- make clean debug release
fi
run-clang-tidy -quiet $cppfiles
