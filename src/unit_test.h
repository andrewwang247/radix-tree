/*
Unit testing trie functionality.

Copyright 2026. Andrew Wang.
*/
#pragma once

#include "trie.h"

namespace rt::unit_test {

/**
 * @brief Get a trie with random insertion order.
 * @return A trie containing all SORTED_WORDS.
 */
trie get_trie();

void concepts();
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

}  // namespace rt::unit_test
