/*
Copyright 2026. Andrew Wang.

Interface for performance testing.
*/
#pragma once
#include <algorithm>
#include <chrono>
#include <iostream>
#include <set>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "trie.h"
using timeunit_t = std::chrono::nanoseconds;

/**
 * @brief Generic return type with a container and the duration to create it.
 */
template <typename Container>
struct timed_container {
  Container container;
  timeunit_t duration;

  /**
   * @brief Construct a new container with an associated duration.
   * @param cont The container to store.
   * @param dur The input duration.
   */
  timed_container(const Container&& cont, timeunit_t dur);
};

namespace perf_test {
// Returns a Container with the words in word_list.
template <typename Container>
timed_container<Container> insert(const std::vector<std::string>& word_list);

// Prefix counting test.
timeunit_t count(const std::set<std::string>& words);
timeunit_t count(const Trie& words);

// Prefix finding text.
timeunit_t find(const std::set<std::string>& words, const std::string& prefix);
timeunit_t find(const Trie& words, const std::string& prefix);

// Prefix erasing test.
timeunit_t erase(std::set<std::string> words, const std::string& prefix);
timeunit_t erase(Trie words, const std::string& prefix);

// Iteration speed test.
template <typename Container>
timeunit_t iterate(const Container& words);
}  // namespace perf_test

// TEMPLATED IMPLEMENTATIONS

template <typename Container>
timed_container<Container>::timed_container(const Container&& cont,
                                            timeunit_t dur)
    : container(std::move(cont)), duration(dur){};

template <typename Container>
timed_container<Container> perf_test::insert(
    const std::vector<std::string>& word_list) {
  // Make announcement.
  if (std::is_same<Container, std::set<std::string>>::value) {
    std::cout << "Set insertion: ";
  } else if (std::is_same<Container, Trie>::value) {
    std::cout << "Trie insertion: ";
  } else {
    throw std::runtime_error("Container must be either set<string> or Trie.");
  }

  // Time insertion with range constructor.
  const auto t0 = std::chrono::high_resolution_clock::now();
  Container words(word_list.begin(), word_list.end());
  const auto t1 = std::chrono::high_resolution_clock::now();
  std::cout << "inserted " << word_list.size() << " words\n";

  return timed_container<Container>(std::move(words), t1 - t0);
}

template <typename Container>
timeunit_t perf_test::iterate(const Container& words) {
  if (std::is_same<Container, std::set<std::string>>::value) {
    std::cout << "Set iteration: ";
  } else if (std::is_same<Container, Trie>::value) {
    std::cout << "Trie iteration: ";
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
