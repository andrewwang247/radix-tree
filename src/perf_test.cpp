/*
Copyright 2026. Andrew Wang.

Performance testing implementation.
*/
#include "perf_test.h"

#include <algorithm>
#include <fstream>
#include <print>
#include <random>
#include <ranges>
#include <string>
#include <string_view>

#include "benchmark.h"

using std::default_random_engine;
using std::ifstream;
using std::print;
using std::println;
using std::random_device;
using std::string;
using std::string_view;

namespace ranges = std::ranges;

int main() {
  static constexpr auto SAMPLE_SIZE = 2500UZ;
  static constexpr auto ANNOUNCE_TEMPLATE = "{:<18}";

  default_random_engine prng{random_device{}()};  // NOLINT(whitespace/braces)
  const auto words = perf_test::read_words(prng);
  const auto solutions = perf_test::read_solutions(prng);
  const auto sublist = perf_test::sample(words, SAMPLE_SIZE, prng);

  println("--- EXECUTING PERFORMANCE TESTS ---");

  set_perf set_benchmark;
  trie_perf trie_benchmark;

  print(ANNOUNCE_TEMPLATE, "Insert words:");
  perf_test::show_comparison(set_benchmark.insert(words),
                             trie_benchmark.insert(words));

  print(ANNOUNCE_TEMPLATE, "Count prefix:");
  perf_test::show_comparison(set_benchmark.count(solutions),
                             trie_benchmark.count(solutions));

  print(ANNOUNCE_TEMPLATE, "Find prefix:");
  perf_test::show_comparison(set_benchmark.find(solutions),
                             trie_benchmark.find(solutions));

  print(ANNOUNCE_TEMPLATE, "Contains words:");
  perf_test::show_comparison(set_benchmark.contains(sublist),
                             trie_benchmark.contains(sublist));

  print(ANNOUNCE_TEMPLATE, "Forward iterate:");
  perf_test::show_comparison(set_benchmark.forward_iterate(),
                             trie_benchmark.forward_iterate());

  print(ANNOUNCE_TEMPLATE, "Reverse iterate:");
  perf_test::show_comparison(set_benchmark.reverse_iterate(),
                             trie_benchmark.reverse_iterate());

  print(ANNOUNCE_TEMPLATE, "Erase prefix:");
  perf_test::show_comparison(set_benchmark.erase(solutions),
                             trie_benchmark.erase(solutions));

  println("--- FINISHED PERFORMANCE TESTS ---");

  println("--- EXECUTING FINAL VERIFICATION ---");

  const auto& word_set = set_benchmark.peek();
  const auto& word_trie = trie_benchmark.peek();

  if (ranges::equal(word_set, word_trie)) {
    println("Forward ranges match");
  } else {
    println("Forward ranges do not match");
  }

  if (ranges::equal(ranges::reverse_view(word_set),
                    ranges::reverse_view(word_trie))) {
    println("Reverse ranges match");
  } else {
    println("Reverse ranges do not match");
  }

  println("--- FINISHED FINAL VERIFICATION ---");
}

void perf_test::show_comparison(timeunit_t set_time, timeunit_t trie_time) {
  static constexpr auto COMPARE_TEMPLATE =
      "{:>4} was {:4.1f} x faster than {:>4}";
  const auto diff_ratio =
      static_cast<double>(std::max(set_time, trie_time).count()) /
      static_cast<double>(std::min(set_time, trie_time).count());
  if (set_time < trie_time) {
    println(COMPARE_TEMPLATE, "set", diff_ratio, "trie");
  } else {
    println(COMPARE_TEMPLATE, "trie", diff_ratio, "set");
  }
}
