/*
Copyright 2026. Andrew Wang.

Unit and performance tests for Trie.
*/
#include <algorithm>
#include <functional>
#include <iomanip>
#include <ios>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "perf_test.h"
#include "trie.h"
#include "unit_test.h"
#include "util.h"

using std::cout;
using std::equal;
using std::fixed;
using std::function;
using std::ios_base;
using std::runtime_error;
using std::set;
using std::setprecision;
using std::string;
using std::vector;

int main() {
  ios_base::sync_with_stdio(false);
  cout << setprecision(1) << fixed;

  vector<function<bool()>> test_cases{
      unit_test::empty,      unit_test::find,      unit_test::insert,
      unit_test::erase,      unit_test::iterate,   unit_test::copy_move,
      unit_test::comparison, unit_test::arithmetic};

  cout << "--- EXECUTING UNIT TESTS ---\n";

  auto passed = 0;
  for (const auto& test : test_cases) {
    if (test()) {
      ++passed;
      cout << " passed\n";
    } else {
      cout << " failed\n";
    }
  }

  cout << "\nPassed " << passed << " out of " << test_cases.size()
       << " unit tests.\n";
  cout << "--- FINISHED UNIT TESTS ---\n\n";

  const auto master_list = read_words();

  cout << "\n--- EXECUTING PERFORMANCE TEST ---\n";

  // Insert perf
  const auto insert_set_result = perf_test::insert<set<string>>(master_list);
  const auto insert_trie_result = perf_test::insert<Trie>(master_list);
  show_performance_comparison(insert_set_result.second,
                              insert_trie_result.second);
  cout << '\n';

  const auto& word_set = insert_set_result.first;
  const auto& word_trie = insert_trie_result.first;

  // Count perf
  const auto count_set_result = perf_test::count(word_set);
  const auto count_trie_result = perf_test::count(word_trie);
  show_performance_comparison(count_set_result.second,
                              count_trie_result.second);
  cout << '\n';

  const auto& set_counts = count_set_result.first;
  const auto& trie_counts = count_trie_result.first;

  // Find perf
  const auto find_set_result = perf_test::find(word_set, "re");
  const auto find_trie_result = perf_test::find(word_trie, "re");
  show_performance_comparison(find_set_result, find_trie_result);
  cout << '\n';

  // Erase perf
  const auto erase_set_result = perf_test::erase(word_set, "pr");
  const auto erase_trie_result = perf_test::erase(word_trie, "pr");
  show_performance_comparison(erase_set_result, erase_trie_result);
  cout << '\n';

  // Iteration perf
  const auto iterate_set_result = perf_test::iterate(word_set);
  const auto iterate_trie_result = perf_test::iterate(word_trie);
  show_performance_comparison(iterate_set_result, iterate_trie_result);

  cout << "--- FINISHED PERFORMANCE TEST ---\n\n";

  cout << "--- EXECUTING FINAL COMPARISON ---\n";

  cout << "Traversal word match test ";
  const bool words_equal = equal(word_set.begin(), word_set.end(),
                                 word_trie.begin(), word_trie.end());
  cout << (words_equal ? "passed\n" : "failed\n");

  cout << "First letter counts test ";
  const bool counts_equal = equal(set_counts.begin(), set_counts.end(),
                                  trie_counts.begin(), trie_counts.end());
  cout << (counts_equal ? "passed\n" : "failed\n");

  cout << "--- FINISHED FINAL COMPARISON ---\n";
}
