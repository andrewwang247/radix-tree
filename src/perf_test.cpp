/*
Copyright 2026. Andrew Wang.

Performance testing implementation.
*/
#include "perf_test.h"

#include <algorithm>
#include <cstddef>
#include <format>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <random>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

using std::cout;
using std::default_random_engine;
using std::format;
using std::ifstream;
using std::ios_base;
using std::numeric_limits;
using std::random_device;
using std::runtime_error;
using std::string;
using std::string_view;
using std::vector;

namespace ranges = std::ranges;
namespace views = std::views;

int main() {
  ios_base::sync_with_stdio(false);
  static constexpr size_t SAMPLE_SIZE = 2500;
  static constexpr auto ANNOUNCE_TEMPLATE = "{:<18}";

  default_random_engine prng{random_device{}()};  // NOLINT(whitespace/braces)
  const auto words = perf_test::read_words(prng);
  const auto solutions = perf_test::read_solutions(prng);
  const auto sublist = perf_test::sample(words, SAMPLE_SIZE, prng);

  cout << "--- EXECUTING PERFORMANCE TEST ---\n";

  set_perf set_benchmark;
  trie_perf trie_benchmark;

  cout << format(ANNOUNCE_TEMPLATE, "Insert words:");
  perf_test::show_comparison(set_benchmark.insert(words),
                             trie_benchmark.insert(words));

  cout << format(ANNOUNCE_TEMPLATE, "Count prefix:");
  perf_test::show_comparison(set_benchmark.count(solutions),
                             trie_benchmark.count(solutions));

  cout << format(ANNOUNCE_TEMPLATE, "Find prefix:");
  perf_test::show_comparison(set_benchmark.find(solutions),
                             trie_benchmark.find(solutions));

  cout << format(ANNOUNCE_TEMPLATE, "Contains words:");
  perf_test::show_comparison(set_benchmark.contains(sublist),
                             trie_benchmark.contains(sublist));

  cout << format(ANNOUNCE_TEMPLATE, "Forward iterate:");
  perf_test::show_comparison(set_benchmark.forward_iterate(),
                             trie_benchmark.forward_iterate());

  cout << format(ANNOUNCE_TEMPLATE, "Reverse iterate:");
  perf_test::show_comparison(set_benchmark.reverse_iterate(),
                             trie_benchmark.reverse_iterate());

  cout << format(ANNOUNCE_TEMPLATE, "Erase prefix:");
  perf_test::show_comparison(set_benchmark.erase(solutions),
                             trie_benchmark.erase(solutions));

  cout << "--- FINISHED PERFORMANCE TEST ---\n";

  cout << "--- EXECUTING FINAL VERIFICATION ---\n";

  const auto& word_set = set_benchmark.peek();
  const auto& word_trie = trie_benchmark.peek();

  if (ranges::equal(word_set, word_trie)) {
    cout << "Forward ranges match\n";
  } else {
    throw runtime_error("Forward ranges do not match");
  }

  if (ranges::equal(ranges::reverse_view(word_set),
                    ranges::reverse_view(word_trie))) {
    cout << "Reverse ranges match\n";
  } else {
    throw runtime_error("Reverse ranges do not match");
  }

  cout << "--- FINISHED FINAL VERIFICATION ---\n";
}

string set_perf::lexicographic_increment(string word) {
  constexpr auto max_char = numeric_limits<decltype(word)::value_type>::max();
  const auto last_non_max = ranges::find_if_not(
      word | views::reverse, [max_char](auto c) { return c == max_char; });
  // All characters are max char. Append min char.
  if (last_non_max == word.rend()) {
    constexpr auto min_char = numeric_limits<decltype(word)::value_type>::min();
    return word + min_char;
  }
  // Increment the last non max char and remove everything after.
  ++*last_non_max;
  word.erase(last_non_max.base(), word.end());
  return word;
}

ranges::range auto set_perf::prefix_range_for(string_view prefix) const {
  // Find the first item that's a prefix
  const auto begin = words.lower_bound(prefix);
  // Find where it stops being a prefix.
  const auto right_bound = lexicographic_increment(string{prefix});
  const auto end = words.lower_bound(right_bound);
  return ranges::subrange{begin, end};
}

timeunit_t set_perf::count(
    const vector<perf_test::solution_t>& solutions) const {
  return count_impl(solutions, [this](string_view prf) {
    return ranges::distance(prefix_range_for(prf));
  });
}

timeunit_t set_perf::find(
    const vector<perf_test::solution_t>& solutions) const {
  return find_impl(solutions,
                   [this](string_view prf) { return prefix_range_for(prf); });
}

timeunit_t set_perf::erase(const vector<perf_test::solution_t>& solutions) {
  size_t total_erased = 0;

  const auto t0 = perf_clock::now();
  for (const auto& [prefix, num, _1, _2] : solutions) {
    const auto prefix_range = prefix_range_for(prefix);
    words.erase(prefix_range.begin(), prefix_range.end());
    total_erased += num;
  }
  const auto t1 = perf_clock::now();

  const auto expected_size = perf_test::WORDS_SIZE - total_erased;
  if (words.size() != expected_size) {
    throw runtime_error(format("Expected {} words after erasing but was {}",
                               expected_size, words.size()));
  }
  return t1 - t0;
}

timeunit_t trie_perf::count(
    const vector<perf_test::solution_t>& solutions) const {
  return count_impl(solutions,
                    [this](string_view prf) { return words.size(prf); });
}

timeunit_t trie_perf::find(
    const vector<perf_test::solution_t>& solutions) const {
  return find_impl(solutions, [this](string_view prf) {
    return ranges::subrange{words.begin(prf), words.end(prf)};
  });
}

timeunit_t trie_perf::erase(const vector<perf_test::solution_t>& solutions) {
  size_t total_erased = 0;

  const auto t0 = perf_clock::now();
  for (const auto& [prefix, num, _1, _2] : solutions) {
    words.erase_prefix(prefix);
    total_erased += num;
  }
  const auto t1 = perf_clock::now();

  const auto expected_size = perf_test::WORDS_SIZE - total_erased;
  if (words.size() != expected_size) {
    throw runtime_error(format("Expected {} words after erasing but was {}",
                               expected_size, words.size()));
  }
  return t1 - t0;
}

void perf_test::show_comparison(timeunit_t set_time, timeunit_t trie_time) {
  static constexpr auto COMPARE_TEMPLATE =
      "{:>4} was {:4.1f} x faster than {:>4}\n";
  if (set_time < trie_time) {
    const auto diff_ratio = static_cast<double>(trie_time.count()) /
                            static_cast<double>(set_time.count());
    cout << format(COMPARE_TEMPLATE, "set", diff_ratio, "trie");
  } else {
    const auto diff_ratio = static_cast<double>(set_time.count()) /
                            static_cast<double>(trie_time.count());
    cout << format(COMPARE_TEMPLATE, "trie", diff_ratio, "set");
  }
}
