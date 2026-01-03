/*
Copyright 2026. Andrew Wang.

Interface for unit testing.
*/
#pragma once

#include <string>
#include <vector>

namespace unit_test {

static const std::vector<std::string> SORTED_WORDS{
    "compute",  "computer", "contain",  "contaminate", "corn",
    "corner",   "mahjong",  "mahogany", "mat",         "material",
    "maternal", "math",     "matrix"};

void empty_single();
void find();
void insert();
void erase();
void forward_iterate();
void reverse_iterate();
void copy_move();
void comparison();
void arithmetic();

void run_all();
}  // namespace unit_test
