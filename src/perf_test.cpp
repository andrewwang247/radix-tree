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
#include <random>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>

#include "benchmark.h"

using std::cout;
using std::default_random_engine;
using std::format;
using std::ifstream;
using std::ios_base;
using std::random_device;
using std::runtime_error;
using std::string;
using std::string_view;

namespace ranges = std::ranges;

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

void perf_test::show_comparison(timeunit_t set_time, timeunit_t trie_time) {
  static constexpr auto COMPARE_TEMPLATE =
      "{:>4} was {:4.1f} x faster than {:>4}\n";
  const auto diff_ratio =
      static_cast<double>(std::max(set_time, trie_time).count()) /
      static_cast<double>(std::min(set_time, trie_time).count());
  if (set_time < trie_time) {
    cout << format(COMPARE_TEMPLATE, "set", diff_ratio, "trie");
  } else {
    cout << format(COMPARE_TEMPLATE, "trie", diff_ratio, "set");
  }
}
