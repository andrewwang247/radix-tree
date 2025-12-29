/*
Copyright 2026. Andrew Wang.

Performance testing implementation.
*/
#include "perf_test.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <iterator>
#include <set>
#include <string>
#include <vector>

using std::count_if;
using std::cout;
using std::distance;
using std::lower_bound;
using std::mismatch;
using std::runtime_error;
using std::set;
using std::string;
using std::vector;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;

constexpr size_t ALPHABET_SIZE = 26;

bool is_prefix(const string& prf, const string& word) {
  // The empty string is a prefix for every string.
  if (prf.empty()) return true;
  // Assuming non-emptiness of prf, it cannot be longer than word.
  if (prf.length() > word.length()) return false;
  // std::algorithm function that returns iterators to first mismatch.
  const auto res = mismatch(prf.begin(), prf.end(), word.begin());

  // If we reached the end of prf, it's a prefix.
  return res.first == prf.end();
}

timeunit_t perf_test::count(const set<string>& words) {
  cout << "Set first letter counts: ";

  vector<set<string>::iterator> bounds;
  bounds.reserve(ALPHABET_SIZE + 1);
  vector<decltype(bounds)::difference_type> distances;
  distances.reserve(ALPHABET_SIZE);

  const auto t0 = high_resolution_clock::now();
  // Get starting iterators on each character.
  for (char c = 'a'; c <= 'z'; ++c) {
    const auto first_of_letter =
        lower_bound(words.begin(), words.end(), string(1, c));
    bounds.push_back(first_of_letter);
  }
  bounds.push_back(words.end());
  // Compute distances between bounds.
  for (size_t i = 0; i < bounds.size() - 1; ++i) {
    const auto dist = distance(bounds[i], bounds[i + 1]);
    distances.push_back(dist);
  }
  const auto t1 = high_resolution_clock::now();

  for (const auto dist : distances) {
    cout << dist << ' ';
  }
  cout << '\n';

  return t1 - t0;
}

timeunit_t perf_test::count(const Trie& words) {
  cout << "Trie first letter counts: ";

  vector<size_t> distances;
  distances.reserve(ALPHABET_SIZE);

  const auto t0 = high_resolution_clock::now();
  for (char c = 'a'; c <= 'z'; ++c) {
    const auto size = words.size(string(1, c));
    distances.push_back(size);
  }
  const auto t1 = high_resolution_clock::now();

  for (const auto dist : distances) {
    cout << dist << ' ';
  }
  cout << '\n';

  return t1 - t0;
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
