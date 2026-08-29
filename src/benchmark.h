/*
Copyright 2026. Andrew Wang.

Benchmarking class interfaces.
*/
#pragma once

#include <algorithm>
#include <array>
#include <chrono>
#include <concepts>
#include <cstddef>
#include <format>
#include <functional>
#include <iostream>
#include <iterator>
#include <limits>
#include <numeric>
#include <random>
#include <ranges>
#include <set>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "perf_test.h"

using perf_clock = std::chrono::steady_clock;

/**
 * Interface for performance testing.
 */
template <std::ranges::bidirectional_range Container>
class perf {
 protected:
  Container words;

 public:
  virtual ~perf() = default;

  /**
   * @brief Expose underlying container.
   * @return Const reference to container.
   */
  const Container& peek() const noexcept;

  /**
   * @brief Construct and insert words into container.
   * @param word_list The full list of words.
   * @return Filled container and elapsed time.
   */
  timeunit_t insert(std::span<const std::string> word_list);

  /**
   * @brief Count number of words with given prefixes.
   * @param solutions The prefixes to count.
   * @return The elapsed time.
   */
  virtual timeunit_t count(
      std::span<const perf_test::solution_t> solutions) const = 0;

  /**
   * @brief Find begin and end range of given prefixes.
   * @param solutions The prefixes to find.
   * @return The elapsed time.
   */
  virtual timeunit_t find(
      std::span<const perf_test::solution_t> solutions) const = 0;

  /**
   * @brief Check for containment of words.
   * @param word_list The words to check.
   * @return The elapsed time.
   */
  timeunit_t contains(std::span<const std::string_view> word_list) const;

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
      std::span<const perf_test::solution_t> solutions) = 0;

 protected:
  /**
   * @brief Increment a string to the next possible in lexicographic order.
   * @param word The current string to process.
   * @return The lexicographical earliest string greater than word.
   */
  static std::string lexicographic_increment(std::string_view word);

  /**
   * @brief Helper implementation function for count benchmark.
   * @param solutions The prefixes to count.
   * @param func The specific count function for this type.
   * @return The elapsed time.
   */
  static timeunit_t count_impl(std::span<const perf_test::solution_t> solutions,
                               std::invocable<std::string_view> auto func);

  /**
   * @brief Helper implementation function for find benchmark.
   * @param solutions The prefixes to find.
   * @param func The specific find function for this type.
   * @return The elapsed time.
   */
  static timeunit_t find_impl(std::span<const perf_test::solution_t> solutions,
                              std::invocable<std::string_view> auto func);

  /**
   * @brief Helper implementation function for erase benchmark.
   * @param solutions The prefixes to erase.
   * @param func The specific erase function for this type.
   * @return The elapsed time.
   */
  timeunit_t erase_impl(std::span<const perf_test::solution_t> solutions,
                        std::invocable<std::string_view> auto func) const;
};

/**
 * Perf class template for std::set.
 */
class set_perf final : public perf<std::set<std::string, std::less<>>> {
 private:
  /**
   * @brief Locate boundaries of a prefix range.
   * @param prefix The prefix to locate.
   * @return The range of words with prefix.
   */
  std::ranges::range auto prefix_range_for(std::string_view prefix) const;

 public:
  timeunit_t count(
      std::span<const perf_test::solution_t> solutions) const override;
  timeunit_t find(
      std::span<const perf_test::solution_t> solutions) const override;
  timeunit_t erase(std::span<const perf_test::solution_t> solutions) override;
};

/**
 * Perf class template for trie.
 */
class trie_perf final : public perf<trie> {
 public:
  timeunit_t count(
      std::span<const perf_test::solution_t> solutions) const override;
  timeunit_t find(
      std::span<const perf_test::solution_t> solutions) const override;
  timeunit_t erase(std::span<const perf_test::solution_t> solutions) override;
};

// NON VIRTUAL TEMPLATED IMPLEMENTATIONS

template <std::ranges::bidirectional_range Container>
const Container& perf<Container>::peek() const noexcept {
  return words;
}

template <std::ranges::bidirectional_range Container>
std::string perf<Container>::lexicographic_increment(std::string_view word) {
  std::string owning_word{word};
  constexpr auto max_char = std::numeric_limits<char>::max();
  const auto last_non_max =
      std::ranges::find_if_not(owning_word | std::views::reverse,
                               [max_char](auto c) { return c == max_char; });
  // All characters are max char. Append min char.
  if (last_non_max == owning_word.rend()) {
    constexpr auto min_char = std::numeric_limits<char>::min();
    return owning_word + min_char;
  }
  // Increment the last non max char and remove everything after.
  ++*last_non_max;
  owning_word.erase(last_non_max.base(), owning_word.end());
  return owning_word;
}

template <std::ranges::bidirectional_range Container>
timeunit_t perf<Container>::count_impl(
    std::span<const perf_test::solution_t> solutions,
    std::invocable<std::string_view> auto func) {
  const auto t0 = perf_clock::now();
  const auto distances =
      solutions | std::views::transform(&perf_test::solution_t::prefix) |
      std::views::transform(func) | std::ranges::to<std::vector>();
  const auto t1 = perf_clock::now();

  for (auto&& [expected, actual] : std::views::zip(solutions, distances)) {
    if (std::cmp_not_equal(expected.count, actual)) {
      throw std::runtime_error(
          std::format("Expected {} words with prefix {} but counted {}",
                      expected.count, expected.prefix, actual));
    }
  }
  return t1 - t0;
}

template <std::ranges::bidirectional_range Container>
timeunit_t perf<Container>::find_impl(
    std::span<const perf_test::solution_t> solutions,
    std::invocable<std::string_view> auto func) {
  const auto t0 = perf_clock::now();
  const auto actual_ranges =
      solutions | std::views::transform(&perf_test::solution_t::prefix) |
      std::views::transform(func) | std::ranges::to<std::vector>();
  const auto t1 = perf_clock::now();

  for (auto&& [expected, actual] : std::views::zip(solutions, actual_ranges)) {
    std::string_view exp_beg = expected.begin;
    std::string_view exp_end = expected.end;
    auto act_beg = *actual.begin();
    auto act_end = *actual.end();
    if (exp_beg != act_beg || exp_end != act_end) {
      throw std::runtime_error(std::format(
          "Expected prefix range for {} to be ({}, {}) but was ({}, {})",
          expected.prefix, exp_beg, exp_end, act_beg, act_end));
    }
  }
  return t1 - t0;
}

template <std::ranges::bidirectional_range Container>
timeunit_t perf<Container>::erase_impl(
    std::span<const perf_test::solution_t> solutions,
    std::invocable<std::string_view> auto func) const {
  const auto total_erased = std::ranges::fold_left(
      solutions | std::views::transform(&perf_test::solution_t::count), 0UZ,
      std::plus{});

  const auto t0 = perf_clock::now();
  std::ranges::for_each(solutions, func, &perf_test::solution_t::prefix);
  const auto t1 = perf_clock::now();

  const auto expected = perf_test::WORDS_SIZE - total_erased;
  if (words.size() != expected) {
    throw std::runtime_error(std::format(
        "Expected {} words after erasing but was {}", expected, words.size()));
  }
  return t1 - t0;
}

template <std::ranges::bidirectional_range Container>
timeunit_t perf<Container>::insert(std::span<const std::string> word_list) {
  // Time insertion with range constructor.
  const auto t0 = perf_clock::now();
  for (const auto& word : word_list) {
    words.insert(word);
  }
  const auto t1 = perf_clock::now();
  return t1 - t0;
}

template <std::ranges::bidirectional_range Container>
timeunit_t perf<Container>::contains(
    std::span<const std::string_view> word_list) const {
  static constexpr std::array<const char*, 108U> non_inc{
      "inte", "nonc", "pseu", "unre", "micr", "nons", "nonp", "coun", "hydr",
      "prot", "nond", "reco", "unpr", "nonr", "unin", "inco", "noni", "undi",
      "prea", "ther", "anth", "tetr", "endo", "extr", "neur", "unst", "tric",
      "subc", "indi", "retr", "radi", "nonf", "nont", "unsu", "impe", "chro",
      "unex", "psyc", "nonm", "unse", "irre", "amph", "unpe", "untr", "sulp",
      "colo", "gran", "hemi", "macr", "squa", "unpa", "cata", "ultr", "prei",
      "unsa", "deca", "impr", "mega", "nonv", "medi", "equi", "chlo", "unma",
      "subt", "stri", "carb", "unsh", "dise", "acro", "spir", "unme", "unsp",
      "chor", "brac", "stro", "misa", "hema", "unfo", "outb", "acet", "oste",
      "unch", "afte", "acti", "subp", "heli", "phyt", "rese", "ente", "squi",
      "unmo", "phen", "unen", "resi", "subd", "prer", "prol", "phyl", "unfa",
      "cryp", "unim", "unso", "impa", "magn", "unha", "scra", "hemo", "brea"};
  const auto contains_key = [this](std::string_view key) {
    return words.contains(key);
  };

  const auto t0 = perf_clock::now();
  const auto inc_iter = std::ranges::find_if_not(word_list, contains_key);
  const auto non_iter = std::ranges::find_if(non_inc, contains_key);
  const auto t1 = perf_clock::now();

  if (inc_iter != word_list.end()) {
    throw std::runtime_error(
        std::format("Expected to find {} but did not", *inc_iter));
  }
  if (non_iter != non_inc.end()) {
    throw std::runtime_error(
        std::format("Expected {} to be missing but was not", *non_iter));
  }
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
    throw std::runtime_error(
        std::format("Expected {} elements but iterated over {}",
                    perf_test::WORDS_SIZE, counter));
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
    throw std::runtime_error(
        std::format("Expected {} elements but iterated over {}",
                    perf_test::WORDS_SIZE, counter));
  }
  return t1 - t0;
}
