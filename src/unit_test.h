/*
Unit testing trie functionality.

Copyright 2026. Andrew Wang.
*/
#pragma once
#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <ranges>
#include <string_view>

#include "trie.h"

namespace rt {

namespace unit_test {

/**
 * @brief Get a trie with random insertion order.
 * @return A trie containing all SORTED_WORDS.
 */
trie get_trie();

/**
 * @brief Assert that a range contains exactly given arguments.
 * @param rng The range to validate.
 * @param elements The elements expected in range.
 */
void assert_elements(sv_range auto&& rng,
                     std::initializer_list<std::string_view> elements);

void concepts();
void empty();
void single_empty();
void single_word();
void find_key();
void find_prefix();
void insert();
void erase_key();
void erase_prefix();
void forward_iterate();
void reverse_iterate();
void copy_move();
void comparison();
void arithmetic();
void representation();

}  // namespace unit_test

// TEMPLATED IMPLEMENTATIONS

void unit_test::assert_elements(
    sv_range auto&& rng, std::initializer_list<std::string_view> elements) {
  // Don't have to re-type std::initializer_list<std::string_view> in tests.
  assert(std::ranges::equal(rng, elements));
}

}  // namespace rt
