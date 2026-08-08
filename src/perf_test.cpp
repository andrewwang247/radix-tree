/*
Copyright 2026. Andrew Wang.

Performance testing implementation.
*/
#include "perf_test.h"

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <iterator>
#include <random>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "iterator.h"

using std::cmp_not_equal;
using std::cout;
using std::default_random_engine;
using std::ifstream;
using std::random_device;
using std::runtime_error;
using std::string;
using std::string_view;
using std::vector;

namespace ranges = std::ranges;

ranges::range auto set_perf::prefix_range_for(const string& prefix) const {
  const auto has_prefix = [&prefix](const string_view word) {
    return word.starts_with(prefix);
  };
  // Find the first item that's a prefix
  const auto begin = words.lower_bound(prefix);
  // Find where it stops being a prefix.
  const auto end = ranges::find_if_not(begin, words.end(), has_prefix);
  return ranges::subrange{begin, end};
}

timeunit_t set_perf::count(
    const vector<perf_test::solution_t>& solutions) const {
  const auto t0 = perf_clock::now();
  for (const auto& [prefix, expected_count, _1, _2] : solutions) {
    const auto prefix_range = prefix_range_for(prefix);
    const auto total = ranges::distance(prefix_range);
    if (cmp_not_equal(expected_count, total)) {
      throw runtime_error("Count does not match solution");
    }
  }
  const auto t1 = perf_clock::now();
  return t1 - t0;
}

timeunit_t set_perf::find(
    const vector<perf_test::solution_t>& solutions) const {
  vector<iter_t> actual_begins;
  vector<iter_t> actual_ends;
  actual_begins.reserve(solutions.size());
  actual_ends.reserve(solutions.size());

  const auto t0 = perf_clock::now();
  for (const auto& solution : solutions) {
    const auto prefix_range = prefix_range_for(solution.prefix);
    actual_begins.emplace_back(prefix_range.begin());
    actual_ends.emplace_back(prefix_range.end());
  }
  const auto t1 = perf_clock::now();

  if (!ranges::equal(solutions | begin_view, actual_begins | deref_view) ||
      !ranges::equal(solutions | end_view, actual_ends | deref_view)) {
    throw runtime_error("Prefix range does not match solution");
  }
  return t1 - t0;
}

timeunit_t set_perf::erase(const vector<perf_test::solution_t>& solutions) {
  const auto t0 = perf_clock::now();
  for (const auto& solution : solutions) {
    const auto prefix_range = prefix_range_for(solution.prefix);
    words.erase(prefix_range.begin(), prefix_range.end());
  }
  const auto t1 = perf_clock::now();
  return t1 - t0;
}

timeunit_t trie_perf::count(
    const vector<perf_test::solution_t>& solutions) const {
  const auto t0 = perf_clock::now();
  for (const auto& [prefix, expected_count, _1, _2] : solutions) {
    const auto total = words.size(prefix);
    if (cmp_not_equal(expected_count, total)) {
      throw runtime_error("Count does not match solution");
    }
  }
  const auto t1 = perf_clock::now();
  return t1 - t0;
}

timeunit_t trie_perf::find(
    const vector<perf_test::solution_t>& solutions) const {
  vector<iterator> actual_begins;
  vector<iterator> actual_ends;
  actual_begins.reserve(solutions.size());
  actual_ends.reserve(solutions.size());

  const auto t0 = perf_clock::now();
  for (const auto& solution : solutions) {
    const auto begin = words.begin(solution.prefix);
    const auto end = words.end(solution.prefix);
    actual_begins.emplace_back(begin);
    actual_ends.emplace_back(end);
  }
  const auto t1 = perf_clock::now();

  if (!ranges::equal(solutions | begin_view, actual_begins | deref_view) ||
      !ranges::equal(solutions | end_view, actual_ends | deref_view)) {
    throw runtime_error("Prefix range does not match solution");
  }
  return t1 - t0;
}

timeunit_t trie_perf::erase(const vector<perf_test::solution_t>& solutions) {
  const auto t0 = perf_clock::now();
  for (const auto& solution : solutions) {
    words.erase_prefix(solution.prefix);
  }
  const auto t1 = perf_clock::now();
  return t1 - t0;
}

/**
 * @brief Display performance comparison between set and Trie operations.
 * @param set_time The time taken by the set.
 * @param trie_time The time taken by the Trie.
 */
static void show_performance_comparison(timeunit_t set_time,
                                        timeunit_t trie_time) {
  if (set_time < trie_time) {
    const auto diff = static_cast<double>(trie_time.count()) /
                      static_cast<double>(set_time.count());
    cout << "set was " << diff << " x faster than trie\n";
  } else {
    const auto diff = static_cast<double>(set_time.count()) /
                      static_cast<double>(trie_time.count());
    cout << "trie was " << diff << " x faster than set\n";
  }
}

vector<string> perf_test::read_words() {
  static random_device rdev;
  vector<string> words;
  words.reserve(WORDS_SIZE);

  ifstream fin{WORDS_FILE};
  if (!fin) throw runtime_error("Could not open words list");
  for (string word; fin >> word;) {
    words.push_back(word);
  }
  ranges::shuffle(words, default_random_engine{rdev()});

  cout << "Imported " << words.size() << " randomly shuffled words\n";
  return words;
}

vector<perf_test::solution_t> perf_test::read_solutions() {
  static random_device rdev;
  vector<solution_t> solutions;
  solutions.reserve(SOLUTIONS_SIZE);

  ifstream fin{SOLUTIONS_FILE};
  if (!fin) throw runtime_error("Could not open solutions file");
  size_t count = 0;
  for (string word, begin, end; fin >> word >> count >> begin >> end;) {
    solutions.emplace_back(word, count, begin, end);
  }
  ranges::shuffle(solutions, default_random_engine{rdev()});

  cout << "Imported " << solutions.size() << " randomly shuffled solutions\n";
  return solutions;
}

void perf_test::run_all() {
  const auto words = read_words();
  const auto solutions = read_solutions();

  cout << "--- EXECUTING PERFORMANCE TEST ---\n";

  set_perf set_benchmark;
  trie_perf trie_benchmark;

  cout << "Insert words: ";
  const auto insert_set_result = set_benchmark.insert(words);
  const auto insert_trie_result = trie_benchmark.insert(words);
  show_performance_comparison(insert_set_result, insert_trie_result);

  cout << "Count prefix: ";
  const auto count_set_result = set_benchmark.count(solutions);
  const auto count_trie_result = trie_benchmark.count(solutions);
  show_performance_comparison(count_set_result, count_trie_result);

  cout << "Find prefix: ";
  const auto find_set_result = set_benchmark.find(solutions);
  const auto find_trie_result = trie_benchmark.find(solutions);
  show_performance_comparison(find_set_result, find_trie_result);

  cout << "Forward iterate: ";
  const auto forward_iterate_set_result = set_benchmark.forward_iterate();
  const auto forward_iterate_trie_result = trie_benchmark.forward_iterate();
  show_performance_comparison(forward_iterate_set_result,
                              forward_iterate_trie_result);

  cout << "Reverse iterate: ";
  const auto reverse_iterate_set_result = set_benchmark.reverse_iterate();
  const auto reverse_iterate_trie_result = trie_benchmark.reverse_iterate();
  show_performance_comparison(reverse_iterate_set_result,
                              reverse_iterate_trie_result);

  cout << "Erase prefix: ";
  const auto erase_set_result = set_benchmark.erase(solutions);
  const auto erase_trie_result = trie_benchmark.erase(solutions);
  show_performance_comparison(erase_set_result, erase_trie_result);

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
