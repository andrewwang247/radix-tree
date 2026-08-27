#!/bin/bash

# Copyright 2026. Andrew Wang.
# Run static checks on all C++ code.

set -uo pipefail

# List of all C++ files.
hfiles="src/*.h"
cppfiles="src/*.cpp"

printf "Running clang-format...\n\n"
clang-format -i -style=file $hfiles $cppfiles

printf "Running cppcheck...\n\n"
cppcheck --language=c++ --std=c++20 --quiet \
    --check-level=exhaustive \
    --enable=all \
    --template=gcc \
    --suppress=checkersReport \
    --suppress=missingIncludeSystem \
    --suppress=unusedStructMember \
    $hfiles $cppfiles

printf "\nRunning cpplint...\n\n"
cpplint --filter=-build/include_subdir --quiet $hfiles $cppfiles

printf "\nRunning clang-tidy...\n\n"
if [[ ! -f "compile_commands.json" ]]; then
    printf "Generating compile commands\n"
    bear -- make clean debug release
fi
clang-tidy $cppfiles
