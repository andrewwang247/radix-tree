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
#include <iterator>
#include <random>
#include <ranges>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "trie.h"

using timeunit_t = std::chrono::nanoseconds;
using perf_clock = std::chrono::steady_clock;

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
  size_t count;
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

/**
 * Interface for performance testing.
 */
template <std::ranges::bidirectional_range Container>
class perf {
 protected:
  static constexpr auto deref = [](std::input_iterator auto iter) {
    return *iter;
  };

  Container words;

 public:
  virtual ~perf() = default;

  /**
   * @brief Expose underlying container.
   * @return Const reference to container.
   */
  const Container& peek() const;

  /**
   * @brief Construct and insert words into container.
   * @param word_list The full list of words.
   * @return Filled container and elapsed time.
   */
  timeunit_t insert(const std::vector<std::string>& word_list);

  /**
   * @brief Count number of words with given prefixes.
   * @param solutions The prefixes to count.
   * @return The elapsed time.
   */
  virtual timeunit_t count(
      const std::vector<perf_test::solution_t>& solutions) const = 0;

  /**
   * @brief Find begin and end range of given prefixes.
   * @param solutions The prefixes to find.
   * @return The elapsed time.
   */
  virtual timeunit_t find(
      const std::vector<perf_test::solution_t>& solutions) const = 0;

  /**
   * @brief Check for containment of words.
   * @param word_list The words to check.
   * @return The elapsed time.
   */
  virtual timeunit_t contains(
      const std::vector<std::string_view>& word_list) const = 0;

  /**
   * @brief Iterate forward over all words.
   * @return The elapsed time.
   */
  timeunit_t forward_iterate() const;

  /**
   * @brief Iterate backwards over all words.
   * @return The elapsed time.
   */
  timeunit_t reverse_iterate() const;

  /**
   * @brief Erase all words with given prefixes.
   * @param solutions The prefixes to erase.
   * @return The elapsed time.
   */
  virtual timeunit_t erase(
      const std::vector<perf_test::solution_t>& solutions) = 0;
};

/**
 * Perf class template for std::set.
 */
class set_perf final : public perf<std::set<std::string>> {
 private:
  using iter_t = decltype(words)::const_iterator;

  /**
   * @brief Increment a string to the next possible in lexicographic order.
   * @param word The current string to process.
   * @return The lexicographical earliest string greater than word.
   */
  static std::string lexicographic_increment(std::string word);

  /**
   * @brief Locate boundaries of a prefix range.
   * @param prefix The prefix to locate.
   * @return The range of words with prefix.
   */
  std::ranges::range auto prefix_range_for(const std::string& prefix) const;

 public:
  timeunit_t count(
      const std::vector<perf_test::solution_t>& solutions) const override;
  timeunit_t find(
      const std::vector<perf_test::solution_t>& solutions) const override;
  timeunit_t contains(
      const std::vector<std::string_view>& word_list) const override;
  timeunit_t erase(
      const std::vector<perf_test::solution_t>& solutions) override;
};

/**
 * Perf class template for trie.
 */
class trie_perf final : public perf<trie> {
 public:
  timeunit_t count(
      const std::vector<perf_test::solution_t>& solutions) const override;
  timeunit_t find(
      const std::vector<perf_test::solution_t>& solutions) const override;
  timeunit_t contains(
      const std::vector<std::string_view>& word_list) const override;
  timeunit_t erase(
      const std::vector<perf_test::solution_t>& solutions) override;
};

// NON VIRTUAL TEMPLATED IMPLEMENTATIONS

std::vector<std::string> perf_test::read_words(
    std::uniform_random_bit_generator auto& prng) {
  std::vector<std::string> words;
  words.reserve(WORDS_SIZE);

  std::ifstream fin{WORDS_FILE};
  if (!fin) throw std::runtime_error("Could not open words list");
  for (std::string word; fin >> word;) {
    words.push_back(word);
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
  if (!fin) throw std::runtime_error("Could not open solutions file");
  size_t count = 0;
  for (std::string word, begin, end; fin >> word >> count >> begin >> end;) {
    solutions.emplace_back(word, count, begin, end);
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

template <std::ranges::bidirectional_range Container>
const Container& perf<Container>::peek() const {
  return words;
}

template <std::ranges::bidirectional_range Container>
timeunit_t perf<Container>::insert(const std::vector<std::string>& word_list) {
  // Time insertion with range constructor.
  const auto t0 = perf_clock::now();
  for (const auto& word : word_list) {
    words.insert(word);
  }
  const auto t1 = perf_clock::now();
  return t1 - t0;
}

template <std::ranges::bidirectional_range Container>
timeunit_t perf<Container>::forward_iterate() const {
  const auto t0 = perf_clock::now();
  // Avoid ranges::distance to prevent size check optimization.
  // We actually want to iterate over the entire container.
  const auto counter = std::distance(words.begin(), words.end());
  const auto t1 = perf_clock::now();

  if (perf_test::WORDS_SIZE != counter) {
    throw std::runtime_error("Iterated over incorrect number of elements");
  }
  return t1 - t0;
}

template <std::ranges::bidirectional_range Container>
timeunit_t perf<Container>::reverse_iterate() const {
  const auto t0 = perf_clock::now();
  const auto counter = std::distance(std::make_reverse_iterator(words.end()),
                                     std::make_reverse_iterator(words.begin()));
  const auto t1 = perf_clock::now();

  if (perf_test::WORDS_SIZE != counter) {
    throw std::runtime_error("Iterated over incorrect number of elements");
  }
  return t1 - t0;
}
