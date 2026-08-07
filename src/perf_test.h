/*
Copyright 2026. Andrew Wang.

Interface for performance testing.
*/
#pragma once
#include <algorithm>
#include <chrono>
#include <iterator>
#include <random>
#include <ranges>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include "trie.h"

using timeunit_t = std::chrono::nanoseconds;
using perf_clock = std::chrono::steady_clock;

namespace perf_test {
static std::random_device rdev{};

static constexpr auto WORDS_FILE = "words.txt";
static constexpr size_t WORDS_SIZE = 370105;

static constexpr auto SOLUTIONS_FILE = "solutions.txt";
static constexpr size_t SOLUTIONS_SIZE = 114;

/**
 * @brief Reads words from the WORDS_FILE into a vector of strings. Randomly
 * permutes before returning.
 * @return A vector of strings containing all words from the file.
 */
std::vector<std::string> read_words();

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
 * @return A vector of solutions containing all entries from the file.
 */
std::vector<solution_t> read_solutions();

void run_all();
}  // namespace perf_test

/**
 * Interface for performance testing.
 */
template <std::ranges::bidirectional_range Container>
class perf {
 protected:
  Container words;

  static constexpr auto deref_view = std::views::transform(
      [](std::input_iterator auto iter) { return *iter; });
  static constexpr auto begin_view = std::views::transform(
      [](const perf_test::solution_t& sol) { return sol.begin; });
  static constexpr auto end_view = std::views::transform(
      [](const perf_test::solution_t& sol) { return sol.end; });

 public:
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
   * Locate boundaries of a prefix range.
   * @param prefix The prefix to locate.
   * @return The range of words with prefix.
   */
  std::ranges::range auto prefix_range_for(const std::string& prefix) const;

 public:
  timeunit_t count(
      const std::vector<perf_test::solution_t>& solutions) const override;
  timeunit_t find(
      const std::vector<perf_test::solution_t>& solutions) const override;
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
  timeunit_t erase(
      const std::vector<perf_test::solution_t>& solutions) override;
};

// NON VIRTUAL TEMPLATED IMPLEMENTATIONS

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
