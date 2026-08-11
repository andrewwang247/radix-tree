/*
Copyright 2026. Andrew Wang.

Performance testing implementation.
*/
#include "perf_test.h"

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <random>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "iterator.h"

using std::cout;
using std::default_random_engine;
using std::fixed;
using std::ifstream;
using std::ios_base;
using std::numeric_limits;
using std::random_device;
using std::runtime_error;
using std::setprecision;
using std::string;
using std::string_view;
using std::vector;

namespace ranges = std::ranges;
namespace views = std::views;

int main() {
  ios_base::sync_with_stdio(false);
  cout << setprecision(1) << fixed;

  const auto words = perf_test::read_words();
  const auto solutions = perf_test::read_solutions();

  cout << "--- EXECUTING PERFORMANCE TEST ---\n";

  set_perf set_benchmark;
  trie_perf trie_benchmark;

  cout << "Insert words: ";
  const auto insert_set_result = set_benchmark.insert(words);
  const auto insert_trie_result = trie_benchmark.insert(words);
  perf_test::show_comparison(insert_set_result, insert_trie_result);

  cout << "Count prefix: ";
  const auto count_set_result = set_benchmark.count(solutions);
  const auto count_trie_result = trie_benchmark.count(solutions);
  perf_test::show_comparison(count_set_result, count_trie_result);

  cout << "Find prefix: ";
  const auto find_set_result = set_benchmark.find(solutions);
  const auto find_trie_result = trie_benchmark.find(solutions);
  perf_test::show_comparison(find_set_result, find_trie_result);

  cout << "Forward iterate: ";
  const auto forward_iterate_set_result = set_benchmark.forward_iterate();
  const auto forward_iterate_trie_result = trie_benchmark.forward_iterate();
  perf_test::show_comparison(forward_iterate_set_result,
                             forward_iterate_trie_result);

  cout << "Reverse iterate: ";
  const auto reverse_iterate_set_result = set_benchmark.reverse_iterate();
  const auto reverse_iterate_trie_result = trie_benchmark.reverse_iterate();
  perf_test::show_comparison(reverse_iterate_set_result,
                             reverse_iterate_trie_result);

  cout << "Erase prefix: ";
  const auto erase_set_result = set_benchmark.erase(solutions);
  const auto erase_trie_result = trie_benchmark.erase(solutions);
  perf_test::show_comparison(erase_set_result, erase_trie_result);

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

ranges::range auto set_perf::prefix_range_for(const string& prefix) const {
  // Find the first item that's a prefix
  const auto begin = words.lower_bound(prefix);
  // Find where it stops being a prefix.
  const auto right_bound = lexicographic_increment(prefix);
  const auto end = words.lower_bound(right_bound);
  return ranges::subrange{begin, end};
}

timeunit_t set_perf::count(
    const vector<perf_test::solution_t>& solutions) const {
  vector<size_t> distances;
  distances.reserve(solutions.size());

  const auto t0 = perf_clock::now();
  for (const auto& solution : solutions) {
    const auto prefix_range = prefix_range_for(solution.prefix);
    const auto total = ranges::distance(prefix_range);
    distances.emplace_back(static_cast<size_t>(total));
  }
  const auto t1 = perf_clock::now();

  if (!ranges::equal(solutions | count_view, distances)) {
    throw runtime_error("Prefix count does not match solution");
  }
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
  vector<size_t> distances;
  distances.reserve(solutions.size());

  const auto t0 = perf_clock::now();
  for (const auto& [prefix, expected_count, _1, _2] : solutions) {
    const auto total = words.size(prefix);
    distances.emplace_back(total);
  }
  const auto t1 = perf_clock::now();

  if (!ranges::equal(solutions | count_view, distances)) {
    throw runtime_error("Prefix count does not match solution");
  }
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

void perf_test::show_comparison(timeunit_t set_time, timeunit_t trie_time) {
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
