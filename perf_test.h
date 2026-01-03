/*
Copyright 2026. Andrew Wang.

Interface for performance testing.
*/
#pragma once
#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>
#include <set>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "trie.h"

using timeunit_t = std::chrono::nanoseconds;
constexpr size_t ALPHABET_SIZE = 26;

namespace perf_test {
// Insertion test. Also constructs the container.
template <typename Container>
std::pair<Container, timeunit_t> insert(
    const std::vector<std::string>& word_list);

// Prefix counting test. Returns array of prefix counts for 'a' to 'z'.
std::pair<std::array<size_t, ALPHABET_SIZE>, timeunit_t> count(
    const std::set<std::string>& words);
std::pair<std::array<size_t, ALPHABET_SIZE>, timeunit_t> count(
    const trie& words);

// Prefix finding text.
timeunit_t find(const std::set<std::string>& words, const std::string& prefix);
timeunit_t find(const trie& words, const std::string& prefix);

// Prefix erasing test.
timeunit_t erase(std::set<std::string> words, const std::string& prefix);
timeunit_t erase(trie words, const std::string& prefix);

// Forward iteration test.
template <typename Container>
timeunit_t forward_iterate(const Container& words);

// Reverse iteration test.
timeunit_t reverse_iterate(const std::set<std::string>& words);
timeunit_t reverse_iterate(const trie& words);

void run_all();
}  // namespace perf_test

// TEMPLATED IMPLEMENTATIONS

template <typename Container>
std::pair<Container, timeunit_t> perf_test::insert(
    const std::vector<std::string>& word_list) {
  // Make announcement.
  if (std::is_same<Container, std::set<std::string>>::value) {
    std::cout << "\tSet insertion: ";
  } else if (std::is_same<Container, trie>::value) {
    std::cout << "\tTrie insertion: ";
  } else {
    throw std::runtime_error("Container must be either set<string> or Trie.");
  }

  // Time insertion with range constructor.
  const auto t0 = std::chrono::high_resolution_clock::now();
  Container words(word_list.begin(), word_list.end());
  const auto t1 = std::chrono::high_resolution_clock::now();
  std::cout << "inserted " << word_list.size() << " words\n";

  return std::make_pair(words, t1 - t0);
}

template <typename Container>
timeunit_t perf_test::forward_iterate(const Container& words) {
  if (std::is_same<Container, std::set<std::string>>::value) {
    std::cout << "\tSet forward iteration: ";
  } else if (std::is_same<Container, trie>::value) {
    std::cout << "\tTrie forward iteration: ";
  } else {
    throw std::runtime_error("Container must be either set<string> or Trie.");
  }

  const auto t0 = std::chrono::high_resolution_clock::now();
  // Work-around for compiler flags not using the key variable.
  // Branch prediction should optimize out this call.
  const auto counter = std::count_if(
      words.begin(), words.end(), [](const auto& key) { return !key.empty(); });
  const auto t1 = std::chrono::high_resolution_clock::now();
  std::cout << "finished iterating over " << counter << " words\n";

  return t1 - t0;
}
