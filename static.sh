#!/bin/bash

# Copyright 2026. Andrew Wang.
# Run static checks on all C++ code.

set -Euo pipefail

# List of all C++ files.
cppfiles="*.h *.cpp"

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
cpplint --filter=-build/include_subdir --quiet $cppfiles
