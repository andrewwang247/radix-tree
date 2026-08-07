/*
Copyright 2026. Andrew Wang.

Performance testing implementation.
*/
#include "perf_test.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <random>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using std::cout;
using std::default_random_engine;
using std::distance;
using std::find_if_not;
using std::ifstream;
using std::make_pair;
using std::pair;
using std::runtime_error;
using std::set;
using std::string;
using std::string_view;
using std::vector;

namespace ranges = std::ranges;

timeunit_t set_perf::count(const vector<perf_test::solution>& solutions) const {
  const auto t0 = perf_clock::now();
  for (const auto& solution : solutions) {
    const auto total = ranges::count_if(words, [&solution](const auto& word) {
      return word.starts_with(solution.prefix);
    });
    if (solution.count != static_cast<size_t>(total)) {
      throw runtime_error("Count does not match solution");
    }
  }
  const auto t1 = perf_clock::now();
  return t1 - t0;
}

pair<set_perf::iter_t, set_perf::iter_t> set_perf::find_begin_end(
    string_view prefix) const {
  // Find the first item that's a prefix using lower bound.
  const auto begin = ranges::lower_bound(words, prefix);
  // Find where it stops being a prefix.
  const auto end = find_if_not(begin, words.end(), [&prefix](const auto& word) {
    return word.starts_with(prefix);
  });
  return make_pair(begin, end);
}

timeunit_t set_perf::find(const vector<perf_test::solution>& solutions) const {
  const auto t0 = perf_clock::now();
  for (const auto& solution : solutions) {
    const auto [begin, end] = find_begin_end(solution.prefix);
    if (solution.begin != *begin || solution.end != *end) {
      throw runtime_error("Found range does not match solution");
    }
  }
  const auto t1 = perf_clock::now();
  return t1 - t0;
}

timeunit_t set_perf::erase(string_view prefix) {
  const auto t0 = perf_clock::now();
  const auto [begin, end] = find_begin_end(prefix);
  words.erase(begin, end);
  const auto t1 = perf_clock::now();
  return t1 - t0;
}

timeunit_t trie_perf::count(
    const vector<perf_test::solution>& solutions) const {
  const auto t0 = perf_clock::now();
  for (const auto& solution : solutions) {
    const auto total = words.size(solution.prefix);
    if (solution.count != static_cast<size_t>(total)) {
      throw runtime_error("Count does not match solution");
    }
  }
  const auto t1 = perf_clock::now();
  return t1 - t0;
}

timeunit_t trie_perf::find(const vector<perf_test::solution>& solutions) const {
  const auto t0 = perf_clock::now();
  for (const auto& solution : solutions) {
    const auto begin = words.begin(solution.prefix);
    const auto end = words.end(solution.prefix);
    if (solution.begin != *begin || solution.end != *end) {
      throw runtime_error("Found range does not match solution");
    }
  }
  const auto t1 = perf_clock::now();
  return t1 - t0;
}

timeunit_t trie_perf::erase(string_view prefix) {
  const auto t0 = perf_clock::now();
  words.erase_prefix(prefix);
  const auto t1 = perf_clock::now();
  return t1 - t0;
}

/**
 * @brief Display performance comparison between set and Trie operations.
 * @param set_time The time taken by the set.
 * @param trie_time The time taken by the Trie.
 */
void show_performance_comparison(timeunit_t set_time, timeunit_t trie_time) {
  if (set_time < trie_time) {
    const auto diff = static_cast<double>(trie_time.count()) /
                      static_cast<double>(set_time.count());
    cout << "\tSet was " << diff << " times faster than Trie\n";
  } else {
    const auto diff = static_cast<double>(set_time.count()) /
                      static_cast<double>(trie_time.count());
    cout << "\tTrie was " << diff << " times faster than Set\n";
  }
}

vector<string> perf_test::read_words() {
  default_random_engine rng{rdev()};
  vector<string> words;
  words.reserve(WORD_LIST_SIZE);

  ifstream fin{WORD_LIST_FILE};
  if (!fin) throw runtime_error("Could not open words list");
  for (string word; fin >> word;) {
    words.push_back(word);
  }
  ranges::shuffle(words, rng);

  cout << "Imported " << words.size() << " randomly shuffled words\n";
  return words;
}

vector<perf_test::solution> perf_test::read_solutions() {
  default_random_engine rng{rdev()};
  vector<solution> solutions;
  solutions.reserve(SOLUTIONS_SIZE);

  ifstream fin{SOLUTIONS_FILE};
  if (!fin) throw runtime_error("Could not open solutions file");
  size_t count;
  for (string word, begin, end; fin >> word >> count >> begin >> end;) {
    solutions.emplace_back(word, count, begin, end);
  }
  ranges::shuffle(solutions, rng);

  cout << "Imported " << solutions.size() << " randomly shuffled solutions\n";
  return solutions;
}

void perf_test::run_all() {
  const auto words = read_words();
  const auto solutions = read_solutions();

  cout << "--- EXECUTING PERFORMANCE TEST ---\n";

  set_perf set_benchmark;
  trie_perf trie_benchmark;

  cout << "Insert\n";
  const auto insert_set_result = set_benchmark.insert(words);
  const auto insert_trie_result = trie_benchmark.insert(words);
  show_performance_comparison(insert_set_result, insert_trie_result);

  cout << "Count\n";
  const auto count_set_result = set_benchmark.count(solutions);
  const auto count_trie_result = trie_benchmark.count(solutions);
  show_performance_comparison(count_set_result, count_trie_result);

  cout << "Find\n";
  const auto find_set_result = set_benchmark.find(solutions);
  const auto find_trie_result = trie_benchmark.find(solutions);
  show_performance_comparison(find_set_result, find_trie_result);

  cout << "Forward Iterate\n";
  const auto forward_iterate_set_result = set_benchmark.forward_iterate();
  const auto forward_iterate_trie_result = trie_benchmark.forward_iterate();
  show_performance_comparison(forward_iterate_set_result,
                              forward_iterate_trie_result);

  cout << "Reverse Iterate\n";
  const auto reverse_iterate_set_result = set_benchmark.reverse_iterate();
  const auto reverse_iterate_trie_result = trie_benchmark.reverse_iterate();
  show_performance_comparison(reverse_iterate_set_result,
                              reverse_iterate_trie_result);

  cout << "Erase\n";
  const auto erase_set_result = set_benchmark.erase("sp");
  const auto erase_trie_result = trie_benchmark.erase("sp");
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
