#!/bin/bash

# Copyright 2026. Andrew Wang.
# Run static checks on all C++ code.

set -uo pipefail

# List of all C++ files.
cppfiles="src/*.h src/*.cpp"

printf "Running clang-format...\n\n"
clang-format -i -style=file $cppfiles

printf "Running cppcheck...\n\n"
cppcheck --language=c++ --std=c++17 --quiet \
    --check-level=exhaustive \
    --enable=all \
    --template=gcc \
    --suppress=checkersReport \
    --suppress=missingIncludeSystem \
    --suppress=unusedStructMember \
    $cppfiles

printf "\nRunning cpplint...\n\n"
cpplint --filter=-build/include_subdir,-build/c++11,-runtime/references \
    --quiet $cppfiles
