/*
Copyright 2026. Andrew Wang.

Interface for unit testing.
*/
#pragma once

#include <array>
#include <string_view>

#include "trie.h"
namespace unit_test {
static constexpr auto RESULT_TEMPLATE = "Test {:<20} passed";
// NOLINTBEGIN(whitespace/indent_namespace)
static constexpr auto SORTED_WORDS = std::to_array<std::string_view>(
    {"compute", "computer", "contain", "contaminate", "corn", "corner",
     "mahjong", "mahogany", "mat", "material", "maternal", "math", "matrix"});
// NOLINTEND

/**
 * @brief Get a trie with random insertion order.
 * @return A trie containing all SORTED_WORDS.
 */
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
