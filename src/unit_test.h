/*
Copyright 2026. Andrew Wang.

Interface for unit testing.
*/
#pragma once

#include <string>
#include <vector>

#include "trie.h"
namespace unit_test {
// NOLINTBEGIN(whitespace/indent_namespace)
static const std::vector<std::string> SORTED_WORDS{
    "compute",  "computer", "contain",  "contaminate", "corn",
    "corner",   "mahjong",  "mahogany", "mat",         "material",
    "maternal", "math",     "matrix"};
// NOLINTEND

trie get_trie();

void empty();
void single();
void find();
void insert();
void erase();
void forward_iterate();
void reverse_iterate();
void copy_move();
void comparison();
void arithmetic();
void representation();
}  // namespace unit_test
