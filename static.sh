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
cpplint --filter=-build/include_subdir,-build/c++11,-runtime/references \
    --quiet $hfiles $cppfiles

printf "\nRunning clang-tidy...\n\n"
clang-tidy $cppfiles
