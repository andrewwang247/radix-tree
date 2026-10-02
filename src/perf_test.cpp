/*
Performance testing.

Copyright 2026. Andrew Wang.
*/
#include "perf_test.h"

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <iterator>
#include <print>
#include <random>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#include "set_perf.h"
#include "trie_perf.h"

using std::default_random_engine;
using std::getline;
using std::ifstream;
using std::print;
using std::println;
using std::random_device;
using std::size_t;
using std::string;
using std::string_view;
using std::vector;

namespace ranges = std::ranges;
namespace views = std::views;

int main() {
  using rt::perf_test::show_comparison;

  static constexpr auto NUM_WORDS = 370'105U;
  default_random_engine prng{random_device{}()};  // NOLINT(whitespace/braces)

  auto words = rt::perf_test::read_words("./resources/words.txt", NUM_WORDS);
  auto solutions =
      rt::perf_test::read_solutions("./resources/solutions.csv", 114U);

  ranges::shuffle(words, prng);
  ranges::shuffle(solutions, prng);

  println("--- EXECUTING PERFORMANCE TESTS ---");

  rt::perf_test::set_perf set_bench;
  rt::perf_test::trie_perf trie_bench;
  static constexpr auto ANNOUNCE_TEMPLATE = "{:<18}";

  print(ANNOUNCE_TEMPLATE, "Insert words:");
  show_comparison(set_bench.insert_words(words),
                  trie_bench.insert_words(words));

  print(ANNOUNCE_TEMPLATE, "Count prefix:");
  show_comparison(set_bench.count_prefix(solutions),
                  trie_bench.count_prefix(solutions));

  print(ANNOUNCE_TEMPLATE, "Find prefix:");
  show_comparison(set_bench.find_prefix(solutions),
                  trie_bench.find_prefix(solutions));

  print(ANNOUNCE_TEMPLATE, "Contains prefix:");
  show_comparison(set_bench.contains_prefix(solutions),
                  trie_bench.contains_prefix(solutions));

  print(ANNOUNCE_TEMPLATE, "Bi-dir iterate:");
  show_comparison(set_bench.bi_dir_iterate(NUM_WORDS),
                  trie_bench.bi_dir_iterate(NUM_WORDS));

  print(ANNOUNCE_TEMPLATE, "Erase prefix:");
  show_comparison(set_bench.erase_prefix(solutions),
                  trie_bench.erase_prefix(solutions));

  println("--- COMPLETED PERFORMANCE TESTS ---");

  println("--- EXECUTING FINAL VERIFICATION ---");

  const auto& word_set = set_bench.peek();
  const auto& word_trie = trie_bench.peek();

  println("Forward ranges match: {}", ranges::equal(word_set, word_trie));
  println("Reverse ranges match: {}",
          ranges::equal(word_set | views::reverse, word_trie | views::reverse));

  println("--- COMPLETED FINAL VERIFICATION ---");
}

namespace rt {

vector<string> perf_test::read_words(const char* name, size_t sz) {
  ifstream fin{name};
  if (!fin) throw perf_error("Could not open {}", name);

  vector<string> words;
  words.reserve(sz);
  for (string word; fin >> word;) {
    words.emplace_back(word);
  }

  if (words.size() != sz) {
    throw perf_error("Expected {} words but got {}", sz, words.size());
  }
  println("Imported {} words from {}", sz, name);
  return words;
}

vector<perf_test::solution_t> perf_test::read_solutions(const char* name,
                                                        size_t sz) {
  ifstream fin{name};
  if (!fin) throw perf_error("Could not open {}", name);

  string line;
  getline(fin, line);

  static constexpr auto EXPECTED_HEADER = "prefix,begin,end,count";
  if (line != EXPECTED_HEADER) {
    throw perf_error("Expected header to define columns {} but was {}",
                     EXPECTED_HEADER, line);
  }

  vector<solution_t> solutions;
  solutions.reserve(sz);
  while (getline(fin, line)) {
    auto row = views::split(line, ',') | views::transform([](auto&& rng) {
                 return string_view{rng.begin(), rng.end()};
               });

    const auto splits = ranges::distance(row);
    if (splits != 4)
      throw perf_error("Expected row {} of size 4 but was {}", line, splits);

    auto it = row.begin();
    const auto prefix = *it++;
    const auto begin = *it++;
    const auto end = *it++;
    const auto count = from_str<size_t>(*it);

    solutions.emplace_back(string{prefix}, string{begin}, string{end}, count);
  }

  if (sz != solutions.size()) {
    throw perf_error("Expected {} solutions but got {}", sz, solutions.size());
  }
  println("Imported {} solutions from {}", sz, name);
  return solutions;
}

void perf_test::show_comparison(timeunit_t set_time, timeunit_t trie_time) {
  static constexpr auto COMPARE_TEMPLATE =
      "{:>4} was {:4.1f} x faster than {:>4}";
  const auto diff_ratio =
      static_cast<double>(std::max(set_time, trie_time).count()) /
      static_cast<double>(std::min(set_time, trie_time).count());
  if (set_time < trie_time) {
    println(COMPARE_TEMPLATE, "set", diff_ratio, "trie");
  } else {
    println(COMPARE_TEMPLATE, "trie", diff_ratio, "set");
  }
}

}  // namespace rt
