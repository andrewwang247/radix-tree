/*
Copyright 2026. Andrew Wang.

Performance testing implementation.
*/
#include "perf_test.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>
#include <iterator>
#include <set>
#include <string>
#include <utility>

#include "util.h"

using std::array;
using std::cout;
using std::distance;
using std::equal;
using std::lower_bound;
using std::make_pair;
using std::pair;
using std::set;
using std::string;
using std::chrono::high_resolution_clock;

pair<array<size_t, ALPHABET_SIZE>, timeunit_t> perf_test::count(
    const set<string>& words) {
  cout << "\tSet count: ";

  array<set<string>::iterator, ALPHABET_SIZE + 1> bounds{};
  array<size_t, ALPHABET_SIZE> distances{};

  const auto t0 = high_resolution_clock::now();
  // Get starting iterators on each character.
  for (char c = 'a'; c <= 'z'; ++c) {
    const auto first_of_letter =
        lower_bound(words.begin(), words.end(), string(1, c));
    bounds[static_cast<size_t>(c - 'a')] = first_of_letter;
  }
  bounds[ALPHABET_SIZE] = words.end();
  // Compute distances between bounds.
  for (size_t i = 0; i < bounds.size() - 1; ++i) {
    const auto dist = distance(bounds[i], bounds[i + 1]);
    distances[i] = static_cast<size_t>(dist);
  }
  const auto t1 = high_resolution_clock::now();

  cout << distances.size() << " prefixes\n";
  return make_pair(distances, t1 - t0);
}

pair<array<size_t, ALPHABET_SIZE>, timeunit_t> perf_test::count(
    const Trie& words) {
  cout << "\tTrie count: ";

  array<size_t, ALPHABET_SIZE> distances{};

  const auto t0 = high_resolution_clock::now();
  for (char c = 'a'; c <= 'z'; ++c) {
    const auto size = words.size(string(1, c));
    distances[static_cast<size_t>(c - 'a')] = size;
  }
  const auto t1 = high_resolution_clock::now();

  cout << distances.size() << " prefixes\n";
  return make_pair(distances, t1 - t0);
}

timeunit_t perf_test::find(const set<string>& words, const string& prefix) {
  cout << "\tSet find: ";

  const auto t0 = high_resolution_clock::now();

  const auto start = lower_bound(words.begin(), words.end(), prefix);
  const auto finish = find_if_not(
      start, words.end(),
      [&prefix](const auto& word) { return util::is_prefix(prefix, word); });
  const auto t1 = high_resolution_clock::now();
  cout << "prefix " << prefix << " starts at " << *start << " and ends at "
       << *finish << '\n';

  return t1 - t0;
}

timeunit_t perf_test::find(const Trie& words, const string& prefix) {
  cout << "\tTrie find: ";

  const auto t0 = high_resolution_clock::now();
  const auto start = words.begin(prefix);
  const auto finish = words.end(prefix);
  const auto t1 = high_resolution_clock::now();
  cout << "prefix " << prefix << " starts at " << *start << " and ends at "
       << *finish << '\n';

  return t1 - t0;
}

timeunit_t perf_test::erase(set<string> words, const string& prefix) {
  cout << "\tSet deletion: ";

  const auto t0 = high_resolution_clock::now();

  // Find the first item that's a prefix using lower bound.
  const auto start = lower_bound(words.begin(), words.end(), prefix);
  // Find where it stops being a prefix.
  const auto finish = find_if_not(
      start, words.end(),
      [&prefix](const auto& word) { return util::is_prefix(prefix, word); });
  words.erase(start, finish);
  const auto t1 = high_resolution_clock::now();
  cout << "erased all words with prefix " << prefix << '\n';

  return t1 - t0;
}

timeunit_t perf_test::erase(Trie words, const string& prefix) {
  cout << "\tTrie deletion: ";

  const auto t0 = high_resolution_clock::now();
  words.erase_prefix(prefix);
  const auto t1 = high_resolution_clock::now();
  cout << "erased all words with prefix " << prefix << '\n';

  return t1 - t0;
}

/**
 * @brief Display performance comparison between set and Trie operations.
 * @param set_time The time taken by the set.
 * @param trie_time The time taken by the Trie.
 */
void show_performance_comparison(timeunit_t set_time, timeunit_t trie_time) {
  if (set_time < trie_time) {
    const auto diff = static_cast<double>(trie_time.count()) / set_time.count();
    cout << "Set was " << diff << " times faster than Trie\n";
  } else {
    const auto diff = static_cast<double>(set_time.count()) / trie_time.count();
    cout << "Trie was " << diff << " times faster than Set\n";
  }
}

void perf_test::run_all() {
  const auto master_list = util::read_words();

  cout << "--- EXECUTING PERFORMANCE TEST ---\n";

  // Insert perf
  const auto insert_set_result = perf_test::insert<set<string>>(master_list);
  const auto insert_trie_result = perf_test::insert<Trie>(master_list);
  show_performance_comparison(insert_set_result.second,
                              insert_trie_result.second);

  const auto& word_set = insert_set_result.first;
  const auto& word_trie = insert_trie_result.first;

  // Count perf
  const auto count_set_result = perf_test::count(word_set);
  const auto count_trie_result = perf_test::count(word_trie);
  show_performance_comparison(count_set_result.second,
                              count_trie_result.second);

  const auto& set_counts = count_set_result.first;
  const auto& trie_counts = count_trie_result.first;

  // Find perf
  const auto find_set_result = perf_test::find(word_set, "re");
  const auto find_trie_result = perf_test::find(word_trie, "re");
  show_performance_comparison(find_set_result, find_trie_result);

  // Erase perf
  const auto erase_set_result = perf_test::erase(word_set, "pr");
  const auto erase_trie_result = perf_test::erase(word_trie, "pr");
  show_performance_comparison(erase_set_result, erase_trie_result);

  // Iteration perf
  const auto iterate_set_result = perf_test::iterate(word_set);
  const auto iterate_trie_result = perf_test::iterate(word_trie);
  show_performance_comparison(iterate_set_result, iterate_trie_result);

  cout << "--- FINISHED PERFORMANCE TEST ---\n";

  cout << "--- EXECUTING FINAL VERIFICATION ---\n";

  cout << "Traversed ranges ";
  const bool words_equal = equal(word_set.begin(), word_set.end(),
                                 word_trie.begin(), word_trie.end());
  cout << (words_equal ? "match\n" : "do not match\n");

  cout << "First letter counts ";
  const bool counts_equal = equal(set_counts.begin(), set_counts.end(),
                                  trie_counts.begin(), trie_counts.end());
  cout << (counts_equal ? "match\n" : "do not match\n");

  if (words_equal && counts_equal) {
    cout << "Verification passed\n";
  } else {
    cout << "Verification failed\n";
  }

  cout << "--- FINISHED FINAL VERIFICATION ---\n";
}
