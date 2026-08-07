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
#include <vector>

using std::cout;
using std::default_random_engine;
using std::distance;
using std::find_if_not;
using std::ifstream;
using std::random_device;
using std::runtime_error;
using std::set;
using std::string;
using std::string_view;
using std::vector;

namespace ranges = std::ranges;

set_perf::set_perf() : perf("Set") {}

timeunit_t set_perf::count(string_view prefix) const {
  const auto t0 = perf_clock::now();
  const auto total = ranges::count_if(
      words, [prefix](const auto& word) { return word.starts_with(prefix); });
  const auto t1 = perf_clock::now();

  cout << '\t' << name << " counted " << total << " words starting with "
       << prefix << '\n';
  return t1 - t0;
}

timeunit_t set_perf::find(string_view prefix) const {
  const auto t0 = perf_clock::now();

  // Find the first item that's a prefix using lower bound.
  const auto start = ranges::lower_bound(words, prefix);
  // Find where it stops being a prefix.
  const auto finish = find_if_not(
      start, words.end(),
      [&prefix](const auto& word) { return word.starts_with(prefix); });
  const auto t1 = perf_clock::now();

  cout << '\t' << name << " found " << prefix << " bounded between " << *start
       << " and " << *finish << '\n';
  return t1 - t0;
}

timeunit_t set_perf::erase(string_view prefix) {
  const auto t0 = perf_clock::now();
  // Find the first item that's a prefix using lower bound.
  const auto start = ranges::lower_bound(words, prefix);
  // Find where it stops being a prefix.
  const auto finish = find_if_not(
      start, words.end(),
      [&prefix](const auto& word) { return word.starts_with(prefix); });
  words.erase(start, finish);
  const auto t1 = perf_clock::now();

  cout << '\t' << name << " erased all words with prefix " << prefix << '\n';
  return t1 - t0;
}

trie_perf::trie_perf() : perf("Trie") {}

timeunit_t trie_perf::count(string_view prefix) const {
  const auto t0 = perf_clock::now();
  const auto total = words.size(prefix);
  const auto t1 = perf_clock::now();

  cout << '\t' << name << " counted " << total << " words starting with "
       << prefix << '\n';
  return t1 - t0;
}

timeunit_t trie_perf::find(string_view prefix) const {
  const auto t0 = perf_clock::now();
  const auto start = words.begin(prefix);
  const auto finish = words.end(prefix);
  const auto t1 = perf_clock::now();

  cout << '\t' << name << " found " << prefix << " bounded between " << *start
       << " and " << *finish << '\n';
  return t1 - t0;
}

timeunit_t trie_perf::erase(string_view prefix) {
  const auto t0 = perf_clock::now();
  words.erase_prefix(prefix);
  const auto t1 = perf_clock::now();

  cout << '\t' << name << " erased all words with prefix " << prefix << '\n';
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
  random_device rdev{};
  default_random_engine rng{rdev()};
  vector<string> master_list;
  master_list.reserve(WORD_LIST_SIZE);

  ifstream fin{WORD_LIST_FILE};
  if (!fin) throw runtime_error("Could not open words.txt");
  for (string word; fin >> word;) {
    master_list.push_back(word);
  }
  ranges::shuffle(master_list, rng);

  cout << "Imported " << master_list.size() << " randomly shuffled words\n";
  return master_list;
}

void perf_test::run_all() {
  const auto master_list = read_words();

  cout << "--- EXECUTING PERFORMANCE TEST ---\n";

  set_perf set_benchmark;
  trie_perf trie_benchmark;

  cout << "Insert\n";
  const auto insert_set_result = set_benchmark.insert(master_list);
  const auto insert_trie_result = trie_benchmark.insert(master_list);
  show_performance_comparison(insert_set_result, insert_trie_result);

  cout << "Count\n";
  const auto count_set_result = set_benchmark.count("ca");
  const auto count_trie_result = trie_benchmark.count("ca");
  show_performance_comparison(count_set_result, count_trie_result);

  cout << "Find\n";
  const auto find_set_result = set_benchmark.find("un");
  const auto find_trie_result = trie_benchmark.find("un");
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
