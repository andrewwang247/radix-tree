/*
Copyright 2026. Andrew Wang.

Interface for performance testing.
*/
#pragma once
#include <algorithm>
#include <array>
#include <chrono>
#include <concepts>
#include <iostream>
#include <iterator>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "trie.h"

using timeunit_t = std::chrono::nanoseconds;
using perf_clock = std::chrono::steady_clock;

namespace perf_test {
static constexpr auto WORD_LIST_FILE = "words.txt";
static constexpr size_t WORD_LIST_SIZE = 466474;

/**
 * @brief Reads words from the WORD_LIST_FILE into a vector of strings. Randomly
 * permutes before returning.
 * @return A vector of strings containing all words from the file.
 */
std::vector<std::string> read_words();
void run_all();
}  // namespace perf_test

/**
 * Constrain valid template types to std::set and trie.
 */
template <typename T>
concept set_or_trie =
    std::same_as<std::set<std::string>, T> || std::same_as<trie, T>;

/**
 * Interface for performance testing.
 */
template <set_or_trie Container>
class perf {
 protected:
  Container words;
  std::string name;
  static constexpr size_t ALPHABET_SIZE = 26;

 public:
  /**
   * @brief Construct default initialized container.
   * @param name_in The name to give to this instance.
   */
  explicit perf(const char* name_in);

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
   * @brief Count prefixes for each starting letter.
   * @return Array of prefix counts for 'a' to 'z' and elapsed time.
   */
  virtual std::pair<std::array<size_t, ALPHABET_SIZE>, timeunit_t> count()
      const = 0;

  /**
   * @brief Find begin and end range of given prefix.
   * @param prefix The prefix to find.
   * @return The elapsed time.
   */
  virtual timeunit_t find(std::string_view prefix) const = 0;

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
   * @brief Erase all words with given prefix.
   * @param prefix The prefix to erase.
   * @return The elapsed time.
   */
  virtual timeunit_t erase(std::string_view prefix) = 0;
};

/**
 * Perf class template for std::set.
 */
class set_perf final : public perf<std::set<std::string>> {
 public:
  set_perf();
  std::pair<std::array<size_t, ALPHABET_SIZE>, timeunit_t> count()
      const override;
  timeunit_t find(std::string_view prefix) const override;
  timeunit_t erase(std::string_view prefix) override;
};

/**
 * Perf class template for trie.
 */
class trie_perf final : public perf<trie> {
 public:
  trie_perf();
  std::pair<std::array<size_t, ALPHABET_SIZE>, timeunit_t> count()
      const override;
  timeunit_t find(std::string_view prefix) const override;
  timeunit_t erase(std::string_view prefix) override;
};

// NON VIRTUAL TEMPLATED IMPLEMENTATIONS

template <set_or_trie Container>
perf<Container>::perf(const char* name_in) : name(name_in) {}

template <set_or_trie Container>
const Container& perf<Container>::peek() const {
  return words;
}

template <set_or_trie Container>
timeunit_t perf<Container>::insert(const std::vector<std::string>& word_list) {
  // Time insertion with range constructor.
  const auto t0 = perf_clock::now();
  for (const auto& word : word_list) {
    words.insert(word);
  }
  const auto t1 = perf_clock::now();

  std::cout << '\t' << name << " inserted " << word_list.size() << " words\n";
  return t1 - t0;
}

template <set_or_trie Container>
timeunit_t perf<Container>::forward_iterate() const {
  const auto t0 = perf_clock::now();
  const auto counter = std::distance(words.begin(), words.end());
  const auto t1 = perf_clock::now();

  std::cout << '\t' << name << " iterated over " << counter << " words\n";
  return t1 - t0;
}

template <set_or_trie Container>
timeunit_t perf<Container>::reverse_iterate() const {
  const auto t0 = perf_clock::now();
  const auto counter = std::distance(std::make_reverse_iterator(words.end()),
                                     std::make_reverse_iterator(words.begin()));
  const auto t1 = perf_clock::now();

  std::cout << '\t' << name << " iterated over " << counter << " words\n";
  return t1 - t0;
}
