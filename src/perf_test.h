/*
Copyright 2026. Andrew Wang.

Interface for performance testing.
*/
#pragma once
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <format>
#include <fstream>
#include <iostream>
#include <random>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#include "trie.h"

using timeunit_t = std::chrono::nanoseconds;

namespace perf_test {
static constexpr auto WORDS_FILE = "./resources/words.txt";
static constexpr size_t WORDS_SIZE = 370105;

static constexpr auto SOLUTIONS_FILE = "./resources/solutions.txt";
static constexpr size_t SOLUTIONS_SIZE = 114;

/**
 * @brief Reads words from the WORDS_FILE into a vector of strings. Randomly
 * permutes before returning.
 * @param prng The random engine to use for shuffling words.
 * @return A vector of strings containing all words from the file.
 */
std::vector<std::string> read_words(
    std::uniform_random_bit_generator auto& prng);

/**
 * Solution to finding and counting a prefix.
 */
struct solution_t {
  std::string prefix;
  size_t count = 0;
  std::string begin, end;
};

/**
 * @brief Reads solutions from the SOLUTIONS_FILE into a vector of. Randomly
 * permutes before returning.
 * @param prng The random engine to use for shuffling solution.
 * @return A vector of solutions containing all entries from the file.
 */
std::vector<solution_t> read_solutions(
    std::uniform_random_bit_generator auto& prng);

/**
 * @brief Randomly sample without replacement from the word list.
 * @param word_list The original words to sample from.
 * @param sample_size Number of samples to retrieve.
 * @param prng The random engine to use for sampling.
 * @return A random choosing of sample_size words from the list.
 */
std::vector<std::string_view> sample(
    const std::vector<std::string>& word_list, size_t sample_size,
    std::uniform_random_bit_generator auto& prng);

/**
 * @brief Display performance comparison between set and Trie operations.
 * @param set_time The time taken by the set.
 * @param trie_time The time taken by the Trie.
 */
void show_comparison(timeunit_t set_time, timeunit_t trie_time);
}  // namespace perf_test

// NON VIRTUAL TEMPLATED IMPLEMENTATIONS

std::vector<std::string> perf_test::read_words(
    std::uniform_random_bit_generator auto& prng) {
  std::vector<std::string> words;
  words.reserve(WORDS_SIZE);

  std::ifstream fin{WORDS_FILE};
  if (!fin)
    throw std::runtime_error(std::format("Could not open {}", WORDS_FILE));
  for (std::string word; fin >> word;) {
    words.push_back(word);
  }

  if (WORDS_SIZE != words.size()) {
    throw std::runtime_error(
        std::format("Expected {} words but got {}", WORDS_SIZE, words.size()));
  }
  std::ranges::shuffle(words, prng);
  std::cout << std::format("Imported {} randomly shuffled words\n",
                           words.size());
  return words;
}

std::vector<perf_test::solution_t> perf_test::read_solutions(
    std::uniform_random_bit_generator auto& prng) {
  std::vector<solution_t> solutions;
  solutions.reserve(SOLUTIONS_SIZE);

  std::ifstream fin{SOLUTIONS_FILE};
  if (!fin)
    throw std::runtime_error(std::format("Could not open {}", SOLUTIONS_FILE));
  size_t count = 0;
  for (std::string word, begin, end; fin >> word >> count >> begin >> end;) {
    solutions.emplace_back(word, count, begin, end);
  }

  if (SOLUTIONS_SIZE != solutions.size()) {
    throw std::runtime_error(std::format("Expected {} words but got {}",
                                         SOLUTIONS_SIZE, solutions.size()));
  }
  std::ranges::shuffle(solutions, prng);
  std::cout << std::format("Imported {} randomly shuffled words\n",
                           solutions.size());
  return solutions;
}

std::vector<std::string_view> perf_test::sample(
    const std::vector<std::string>& word_list, size_t sample_size,
    std::uniform_random_bit_generator auto& prng) {
  std::vector<std::string_view> sub_list(sample_size);
  std::ranges::sample(word_list, sub_list.begin(),
                      static_cast<int32_t>(sample_size), prng);
  return sub_list;
}
