/*
Copyright 2026. Andrew Wang.

Interface for unit testing.
*/
#pragma once

#include <random>
#include <string>
#include <vector>

namespace unit_test {
// NOLINTBEGIN(whitespace/indent_namespace)
static const std::vector<std::string> SORTED_WORDS{
    "compute",  "computer", "contain",  "contaminate", "corn",
    "corner",   "mahjong",  "mahogany", "mat",         "material",
    "maternal", "math",     "matrix"};
// NOLINTEND

static std::random_device RANDOM_DEVICE{};
static std::default_random_engine RNG{RANDOM_DEVICE()};

void empty_single();
void find();
void insert();
void erase();
void forward_iterate();
void reverse_iterate();
void copy_move();
void comparison();
void arithmetic();
void representation();

void run_all();
}  // namespace unit_test
