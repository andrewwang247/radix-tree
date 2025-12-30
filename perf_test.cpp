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
using std::lower_bound;
using std::make_pair;
using std::pair;
using std::set;
using std::string;
using std::chrono::high_resolution_clock;

pair<array<size_t, ALPHABET_SIZE>, timeunit_t> perf_test::count(
    const set<string>& words) {
  cout << "Set first letter counts: ";

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

  for (const auto dist : distances) {
    cout << dist << ' ';
  }
  cout << '\n';

  return make_pair(distances, t1 - t0);
}

pair<array<size_t, ALPHABET_SIZE>, timeunit_t> perf_test::count(
    const Trie& words) {
  cout << "Trie first letter counts: ";

  array<size_t, ALPHABET_SIZE> distances{};

  const auto t0 = high_resolution_clock::now();
  for (char c = 'a'; c <= 'z'; ++c) {
    const auto size = words.size(string(1, c));
    distances[static_cast<size_t>(c - 'a')] = size;
  }
  const auto t1 = high_resolution_clock::now();

  for (const auto dist : distances) {
    cout << dist << ' ';
  }
  cout << '\n';

  return make_pair(distances, t1 - t0);
}

timeunit_t perf_test::find(const set<string>& words, const string& prefix) {
  cout << "Set find: ";

  const auto t0 = high_resolution_clock::now();

  const auto start = lower_bound(words.begin(), words.end(), prefix);
  const auto finish = find_if_not(
      start, words.end(),
      [&prefix](const auto& word) { return is_prefix(prefix, word); });
  const auto t1 = high_resolution_clock::now();
  cout << "prefix " << prefix << " starts at " << *start << " and ends at "
       << *finish << '\n';

  return t1 - t0;
}

timeunit_t perf_test::find(const Trie& words, const string& prefix) {
  cout << "Trie find: ";

  const auto t0 = high_resolution_clock::now();
  const auto start = words.begin(prefix);
  const auto finish = words.end(prefix);
  const auto t1 = high_resolution_clock::now();
  cout << "prefix " << prefix << " starts at " << *start << " and ends at "
       << *finish << '\n';

  return t1 - t0;
}

timeunit_t perf_test::erase(set<string> words, const string& prefix) {
  cout << "Set deletion: ";

  const auto t0 = high_resolution_clock::now();

  // Find the first item that's a prefix using lower bound.
  const auto start = lower_bound(words.begin(), words.end(), prefix);
  // Find where it stops being a prefix.
  const auto finish = find_if_not(
      start, words.end(),
      [&prefix](const auto& word) { return is_prefix(prefix, word); });
  words.erase(start, finish);
  const auto t1 = high_resolution_clock::now();
  cout << "erased all words with prefix " << prefix << '\n';

  return t1 - t0;
}

timeunit_t perf_test::erase(Trie words, const string& prefix) {
  cout << "Trie deletion: ";

  const auto t0 = high_resolution_clock::now();
  words.erase_prefix(prefix);
  const auto t1 = high_resolution_clock::now();
  cout << "erased all words with prefix " << prefix << '\n';

  return t1 - t0;
}

void show_performance_comparison(timeunit_t set_time, timeunit_t trie_time) {
  if (set_time < trie_time) {
    const auto diff = static_cast<double>(trie_time.count()) / set_time.count();
    cout << "\tSet was " << diff << " times faster than Trie\n";
  } else {
    const auto diff = static_cast<double>(set_time.count()) / trie_time.count();
    cout << "\tTrie was " << diff << " times faster than Set\n";
  }
}
