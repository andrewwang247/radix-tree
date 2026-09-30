/*
Benchmarking structures.

Copyright 2026. Andrew Wang.
*/
#pragma once
#include <algorithm>
#include <cstddef>
#include <format>
#include <iterator>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "perf_test.h"

namespace rt::perf_test {

/**
 * @brief Interface for performance testing.
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
  virtual timeunit_t count(std::span<const solution_t> solutions) const = 0;

  /**
   * @brief Find key range of given prefixes.
   * @param solutions The prefixes to find.
   * @return The elapsed time.
   */
  virtual timeunit_t find(std::span<const solution_t> solutions) const = 0;

  /**
   * @brief Check for containment of prefixes.
   * @param solutions The prefixes to check.
   * @return The elapsed time.
   */
  virtual timeunit_t contains(std::span<const solution_t> solutions) const = 0;

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
  virtual timeunit_t erase(std::span<const solution_t> solutions) = 0;

 protected:
  /**
   * @brief Helper implementation function for count benchmark.
   * @param solutions The prefixes to count.
   * @param func The specific count function for this type.
   * @return The elapsed time.
   */
  static timeunit_t count_impl(std::span<const solution_t> solutions,
                               std::invocable<std::string_view> auto func);

  /**
   * @brief Helper implementation function for find benchmark.
   * @param solutions The prefixes to find.
   * @param func The specific find function for this type.
   * @return The elapsed time.
   */
  static timeunit_t find_impl(std::span<const solution_t> solutions,
                              std::invocable<std::string_view> auto func);

  /**
   * @brief Helper implementation function for contains benchmark.
   * @param solutions The prefixes to check.
   * @param func The specific contains function for this type.
   * @return The elapsed time.
   */
  static timeunit_t contains_impl(std::span<const solution_t> solutions,
                                  std::invocable<std::string_view> auto func);

  /**
   * @brief Helper implementation function for erase benchmark.
   * @param solutions The prefixes to erase.
   * @param func The specific erase function for this type.
   * @return The elapsed time.
   */
  timeunit_t erase_impl(std::span<const solution_t> solutions,
                        std::invocable<std::string_view> auto func) const;
};

// NON VIRTUAL TEMPLATED IMPLEMENTATIONS

template <std::ranges::bidirectional_range Container>
const Container& perf<Container>::peek() const noexcept {
  return words;
}

template <std::ranges::bidirectional_range Container>
timeunit_t perf<Container>::count_impl(
    std::span<const solution_t> solutions,
    std::invocable<std::string_view> auto func) {
  const auto t0 = perf_clock::now();
  const auto distances =
      solutions | std::views::transform(&solution_t::prefix) |
      std::views::transform(func) | std::ranges::to<std::vector>();
  const auto t1 = perf_clock::now();

  for (auto&& [expected, actual] : std::views::zip(solutions, distances)) {
    if (std::cmp_not_equal(expected.count, actual)) {
      throw perf_error("Expected {} words with prefix {} but counted {}",
                       expected.count, expected.prefix, actual);
    }
  }
  return t1 - t0;
}

template <std::ranges::bidirectional_range Container>
timeunit_t perf<Container>::find_impl(
    std::span<const solution_t> solutions,
    std::invocable<std::string_view> auto func) {
  const auto t0 = perf_clock::now();
  const auto actual_ranges =
      solutions | std::views::transform(&solution_t::prefix) |
      std::views::transform(func) | std::ranges::to<std::vector>();
  const auto t1 = perf_clock::now();

  for (auto&& [expected, actual] : std::views::zip(solutions, actual_ranges)) {
    const auto& [prf, exp_beg, exp_end, _] = expected;
    const auto act_beg = *actual.begin();
    const auto act_end = *actual.end();
    if (exp_beg != act_beg || exp_end != act_end) {
      throw perf_error(
          "Expected prefix range for {} to be ({}, {}) but was ({}, {})", prf,
          exp_beg, exp_end, act_beg, act_end);
    }
  }
  return t1 - t0;
}

template <std::ranges::bidirectional_range Container>
timeunit_t perf<Container>::contains_impl(
    std::span<const solution_t> solutions,
    std::invocable<std::string_view> auto func) {
  const auto t0 = perf_clock::now();
  const auto iter =
      std::ranges::find_if_not(solutions, func, &solution_t::prefix);
  const auto t1 = perf_clock::now();

  if (iter != solutions.end()) {
    throw perf_error("Expected to find prefix {} but did not", iter->prefix);
  }
  return t1 - t0;
}

template <std::ranges::bidirectional_range Container>
timeunit_t perf<Container>::erase_impl(
    std::span<const solution_t> solutions,
    std::invocable<std::string_view> auto func) const {
  const auto total_erased = std::ranges::fold_left(
      solutions | std::views::transform(&solution_t::count), 0UZ, std::plus{});

  const auto t0 = perf_clock::now();
  std::ranges::for_each(solutions, func, &solution_t::prefix);
  const auto t1 = perf_clock::now();

  const auto expected = perf_test::WORDS_SIZE - total_erased;
  if (words.size() != expected) {
    throw perf_error("Expected {} words after erasing but was {}", expected,
                     words.size());
  }
  return t1 - t0;
}

template <std::ranges::bidirectional_range Container>
timeunit_t perf<Container>::insert(std::span<const std::string> word_list) {
  // Time insertion with range constructor.
  const auto t0 = perf_clock::now();
  words.insert_range(word_list);
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

  if (counter != WORDS_SIZE) {
    throw perf_error("Expected {} elements but iterated over {}", WORDS_SIZE,
                     counter);
  }
  return t1 - t0;
}

template <std::ranges::bidirectional_range Container>
timeunit_t perf<Container>::reverse_iterate() const {
  const auto t0 = perf_clock::now();
  const auto counter = std::distance(std::make_reverse_iterator(words.end()),
                                     std::make_reverse_iterator(words.begin()));
  const auto t1 = perf_clock::now();

  if (counter != WORDS_SIZE) {
    throw perf_error("Expected {} elements but iterated over {}", WORDS_SIZE,
                     counter);
  }
  return t1 - t0;
}

}  // namespace rt::perf_test
