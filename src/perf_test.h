/*
Performance testing.

Copyright 2026. Andrew Wang.
*/
#pragma once
#include <charconv>
#include <chrono>
#include <cstddef>
#include <format>
#include <limits>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

#include "trie.h"

namespace rt {

namespace perf_test {

using perf_clock = std::chrono::steady_clock;
using timeunit_t = perf_clock::time_point::duration;

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
 * @brief Parse and convert a string to a different type.
 * @param str The string to convert from.
 * @tparam T the type to convert to.
 * @return The value parsed from str.
 */
template <typename T>
T from_str(std::string_view str);

/**
 * @brief Solution to finding and counting a prefix.
 */
struct solution_t {
  std::string prefix, begin, end;
  std::size_t count{};
};

/**
 * @brief Reads words from the WORDS_FILE into a vector of strings.
 * @param name The file name to read from.
 * @param sz The expected number of entries.
 * @return A vector of strings containing all words from the file.
 */
std::vector<std::string> read_words(const char* name, std::size_t sz);

/**
 * @brief Reads solutions from the SOLUTIONS_FILE into a vector of.
 * @param name The file name to read from.
 * @param sz The expected number of entries.
 * @return A vector of solutions containing all entries from the file.
 */
std::vector<solution_t> read_solutions(const char* name, std::size_t sz);

/**
 * @brief Display performance comparison between set and Trie operations.
 * @param set_time The time taken by the set.
 * @param trie_time The time taken by the Trie.
 */
void show_comparison(timeunit_t set_time, timeunit_t trie_time);

}  // namespace perf_test

// TEMPLATED IMPLEMENTATIONS

template <typename T>
T perf_test::from_str(std::string_view str) {
  T result{};
  const auto [ptr, ec] = std::from_chars(str.begin(), str.end(), result);
  if (ec != std::errc{}) {
    throw std::system_error(std::make_error_code(ec),
                            std::format("Failed to convert {}", str));
  }
  if (ptr != str.end()) {
    const auto up_to = std::string_view{str.begin(), ptr};
    const auto remainder = std::string_view{ptr, str.end()};
    throw perf_error("Could only convert up to {} with remainder {}", up_to,
                     remainder);
  }
  return result;
}

}  // namespace rt
