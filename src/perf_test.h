/*
Copyright 2026. Andrew Wang.

Interface for performance testing.
*/
#pragma once
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <format>
#include <limits>
#include <random>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "trie.h"

using perf_clock = std::chrono::steady_clock;
using timeunit_t = std::chrono::nanoseconds;

/**
 * @brief Error during performance testing.
 */
class perf_error : public std::runtime_error {
 public:
  /**
   * @brief Formatted runtime_error constructor.
   * @param fmt The format string.
   * @param args Arguments to format string.
   */
  template <typename... Args>
  explicit perf_error(std::format_string<Args...> fmt, Args&&... args)
      : std::runtime_error(std::format(fmt, std::forward<Args>(args)...)) {}
};

/**
 * @brief Performance testing.
 */
namespace perf_test {
static constexpr auto WORDS_FILE = "./resources/words.txt";
static constexpr auto WORDS_SIZE = 370'105UZ;

static constexpr auto SOLUTIONS_FILE = "./resources/solutions.csv";
static constexpr auto SOLUTIONS_SIZE = 114UZ;

template <typename T>
concept PRNG = std::uniform_random_bit_generator<std::remove_cvref_t<T>>;

/**
 * @brief Solution to finding and counting a prefix.
 */
struct solution_t {
  std::string prefix, begin, end;
  std::size_t count = 0UZ;
};

/**
 * @brief Reads words from the WORDS_FILE into a vector of strings.
 * @return A vector of strings containing all words from the file.
 */
std::vector<std::string> read_words();

/**
 * @brief Reads solutions from the SOLUTIONS_FILE into a vector of.
 * @return A vector of solutions containing all entries from the file.
 */
std::vector<solution_t> read_solutions();

/**
 * @brief Randomly permute from an original list.
 * @param original The original list to shuffle. Pass by move / elision.
 * @param prng The random bit generator to use from permuting.
 * @return The shuffled original list.
 */
template <typename T>
std::vector<T> permute(std::vector<T> original, PRNG auto&& prng);

/**
 * @brief Randomly sample without replacement from the word list.
 * @param word_list The original words to sample from.
 * @param sample_size Number of samples to retrieve.
 * @param prng The random bit generator to use for sampling.
 * @return A subset of sample_size random string_views into the list.
 */
std::vector<std::string_view> sample(std::span<const std::string> word_list,
                                     std::size_t sample_size, PRNG auto&& prng);

/**
 * @brief Increment a string to the next possible in lexicographic order.
 * @param word The current string to process.
 * @return The lexicographical earliest string greater than word.
 */
constexpr std::string lexicographic_increment(std::string word);

/**
 * @brief Display performance comparison between set and Trie operations.
 * @param set_time The time taken by the set.
 * @param trie_time The time taken by the Trie.
 */
void show_comparison(timeunit_t set_time, timeunit_t trie_time);
}  // namespace perf_test

// CONSTEXPR AND TEMPLATED IMPLEMENTATIONS

template <typename T>
std::vector<T> perf_test::permute(std::vector<T> original, PRNG auto&& prng) {
  std::ranges::shuffle(original, prng);
  return original;
}

std::vector<std::string_view> perf_test::sample(
    std::span<const std::string> word_list, std::size_t sample_size,
    PRNG auto&& prng) {
  std::vector<std::string_view> sub_list(sample_size);
  std::ranges::sample(word_list, sub_list.begin(),
                      static_cast<std::int32_t>(sample_size), prng);
  return sub_list;
}

constexpr std::string perf_test::lexicographic_increment(std::string word) {
  const auto last_non_max =
      word.find_last_not_of(std::numeric_limits<char>::max());
  if (last_non_max != std::string::npos) {
    // Increment last non max char and remove everything after.
    ++word[last_non_max];
    word.erase(last_non_max + 1);
  } else {
    // All characters are max char. Append min char.
    word += std::numeric_limits<char>::min();
  }
  return word;
}
