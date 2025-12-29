/*
Copyright 2026. Andrew Wang.

Unit and performance tests for Trie.
*/
#include <algorithm>
#include <chrono>
#include <fstream>
#include <functional>
#include <iomanip>
#include <ios>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "perf_test.h"
#include "trie.h"
#include "unit_test.h"

using std::cout;
using std::equal;
using std::fixed;
using std::function;
using std::ifstream;
using std::ios_base;
using std::runtime_error;
using std::set;
using std::setprecision;
using std::string;
using std::vector;

/**
 * @brief Reads words from the given file into a vector of strings.
 * @param word_file The path to the word list file.
 * @return A vector of strings containing all words from the file.
 */
vector<string> read_words(const string& word_file);

/**
 * @brief Display performance comparison between set and Trie operations.
 * @param set_time The time taken by the set.
 * @param trie_time The time taken by the Trie.
 */
void show_performance_comparison(timeunit_t set_time, timeunit_t trie_time);

int main() {
  ios_base::sync_with_stdio(false);
  cout << setprecision(2) << fixed;

  vector<function<bool()>> test_cases{
      unit_test::empty,      unit_test::find,      unit_test::insert,
      unit_test::erase,      unit_test::iterate,   unit_test::copy,
      unit_test::comparison, unit_test::arithmetic};

  cout << "--- EXECUTING UNIT TESTS ---\n";

  __uint16_t passed = 0;
  for (const auto& test : test_cases) {
    if (test()) {
      ++passed;
      cout << " passed.\n";
    } else {
      cout << " failed.\n";
    }
  }

  cout << "\nPassed " << passed << " out of " << test_cases.size()
       << " unit tests.\n";
  cout << "--- FINISHED UNIT TESTS ---\n\n";

  // cppcheck-suppress "throwInEntryPoint"
  const auto master_list = read_words("words.txt");

  cout << "\n--- EXECUTING PERFORMANCE TEST ---\n";

  // Insert perf
  const auto insert_set_result = perf_test::insert<set<string>>(master_list);
  const auto insert_trie_result = perf_test::insert<Trie>(master_list);
  show_performance_comparison(insert_set_result.duration,
                              insert_trie_result.duration);
  cout << '\n';

  const auto word_set = std::move(insert_set_result.container);
  const auto word_trie = std::move(insert_trie_result.container);
  // DO NOT REFERENCE MOVED VALUES BEYOND THIS POINT!

  // Count perf
  const auto count_set_result = perf_test::count(word_set);
  const auto count_trie_result = perf_test::count(word_trie);
  show_performance_comparison(count_set_result, count_trie_result);
  cout << '\n';

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

  if (equal(word_set.begin(), word_set.end(), word_trie.begin(),
            word_trie.end()))
    cout << "--- FINAL COMPARISON CHECK PASSED ---\n";
  else
    cout << "--- FINAL COMPARISON CHECK FAILED ---\n";
}

vector<string> read_words(const string& word_file) {
  ifstream fin(word_file);
  if (!fin) throw runtime_error("Could not open words.txt");

  cout << "Reading words.txt...\n";
  vector<string> master_list;
  for (string word; fin >> word;) {
    master_list.push_back(word);
  }
  fin.close();
  cout << "Imported " << master_list.size() << " words\n";
  return master_list;
}

void show_performance_comparison(timeunit_t set_time, timeunit_t trie_time) {
  if (set_time < trie_time) {
    const auto diff = static_cast<double>(trie_time.count()) / set_time.count();
    cout << "\tSet was faster by a factor of " << diff << '\n';
  } else {
    const auto diff = static_cast<double>(set_time.count()) / trie_time.count();
    cout << "\tTrie was faster by a factor of " << diff << '\n';
  }
}
