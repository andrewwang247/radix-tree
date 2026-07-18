/*
Copyright 2026. Andrew Wang.

Performance testing implementation.
*/
#include "perf_test.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using std::array;
using std::cout;
using std::default_random_engine;
using std::distance;
using std::find_if_not;
using std::ifstream;
using std::make_pair;
using std::pair;
using std::random_device;
using std::runtime_error;
using std::set;
using std::string;
using std::vector;
using std::chrono::high_resolution_clock;

namespace ranges = std::ranges;

set_perf::set_perf() : perf("Set") {}

pair<array<size_t, set_perf::ALPHABET_SIZE>, timeunit_t> set_perf::count()
    const {
  array<set<string>::iterator, ALPHABET_SIZE + 1> bounds{};
  array<size_t, ALPHABET_SIZE> distances{};

  const auto t0 = high_resolution_clock::now();
  // Get starting iterators on each character.
  for (char c = 'a'; c <= 'z'; ++c) {
    const auto first_of_letter = ranges::lower_bound(words, string(1, c));
    bounds[static_cast<size_t>(c - 'a')] = first_of_letter;
  }
  bounds[ALPHABET_SIZE] = words.end();
  // Compute distances between bounds.
  for (size_t i = 0; i < bounds.size() - 1; ++i) {
    const auto dist = distance(bounds[i], bounds[i + 1]);
    distances[i] = static_cast<size_t>(dist);
  }
  const auto t1 = high_resolution_clock::now();

  cout << '\t' << name << " counted " << distances.size() << " prefixes\n";
  return make_pair(distances, t1 - t0);
}

timeunit_t set_perf::find(const string& prefix) const {
  const auto t0 = high_resolution_clock::now();

  // Find the first item that's a prefix using lower bound.
  const auto start = ranges::lower_bound(words, prefix);
  // Find where it stops being a prefix.
  const auto finish = find_if_not(
      start, words.end(),
      [&prefix](const auto& word) { return word.starts_with(prefix); });
  const auto t1 = high_resolution_clock::now();

  cout << '\t' << name << " found prefix " << prefix << " bounded between "
       << *start << " and " << *finish << '\n';
  return t1 - t0;
}

timeunit_t set_perf::erase(const string& prefix) {
  const auto t0 = high_resolution_clock::now();
  // Find the first item that's a prefix using lower bound.
  const auto start = ranges::lower_bound(words, prefix);
  // Find where it stops being a prefix.
  const auto finish = find_if_not(
      start, words.end(),
      [&prefix](const auto& word) { return word.starts_with(prefix); });
  words.erase(start, finish);
  const auto t1 = high_resolution_clock::now();

  cout << '\t' << name << " erased all words with prefix " << prefix << '\n';
  return t1 - t0;
}

trie_perf::trie_perf() : perf("Trie") {}

pair<array<size_t, trie_perf::ALPHABET_SIZE>, timeunit_t> trie_perf::count()
    const {
  array<size_t, ALPHABET_SIZE> distances{};

  const auto t0 = high_resolution_clock::now();
  for (char c = 'a'; c <= 'z'; ++c) {
    const auto size = words.size(string(1, c));
    distances[static_cast<size_t>(c - 'a')] = size;
  }
  const auto t1 = high_resolution_clock::now();

  cout << '\t' << name << " counted " << distances.size() << " prefixes\n";
  return make_pair(distances, t1 - t0);
}

timeunit_t trie_perf::find(const string& prefix) const {
  const auto t0 = high_resolution_clock::now();
  const auto start = words.begin(prefix);
  const auto finish = words.end(prefix);
  const auto t1 = high_resolution_clock::now();

  cout << '\t' << name << " found prefix " << prefix << " bounded between "
       << *start << " and " << *finish << '\n';
  return t1 - t0;
}

timeunit_t trie_perf::erase(const string& prefix) {
  const auto t0 = high_resolution_clock::now();
  words.erase_prefix(prefix);
  const auto t1 = high_resolution_clock::now();

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
  const auto count_set_result = set_benchmark.count();
  const auto count_trie_result = trie_benchmark.count();
  show_performance_comparison(count_set_result.second,
                              count_trie_result.second);

  const auto& set_counts = count_set_result.first;
  const auto& trie_counts = count_trie_result.first;

  cout << "Find\n";
  const auto find_set_result = set_benchmark.find("re");
  const auto find_trie_result = trie_benchmark.find("re");
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
  const auto erase_set_result = set_benchmark.erase("pr");
  const auto erase_trie_result = trie_benchmark.erase("pr");
  show_performance_comparison(erase_set_result, erase_trie_result);

  cout << "--- FINISHED PERFORMANCE TEST ---\n";

  cout << "--- EXECUTING FINAL VERIFICATION ---\n";

  const auto& word_set = set_benchmark.peek();
  const auto& word_trie = trie_benchmark.peek();

  cout << "Forward ranges ";
  const bool words_forward_equal = ranges::equal(word_set, word_trie);
  cout << (words_forward_equal ? "match\n" : "do not match\n");

  cout << "Reverse ranges ";
  const bool words_reverse_equal = ranges::equal(
      ranges::reverse_view(word_set), ranges::reverse_view(word_trie));
  cout << (words_reverse_equal ? "match\n" : "do not match\n");

  cout << "First letter counts ";
  const bool counts_equal = ranges::equal(set_counts, trie_counts);
  cout << (counts_equal ? "match\n" : "do not match\n");

  cout << "--- FINISHED FINAL VERIFICATION ---\n";
}
