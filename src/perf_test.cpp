/*
Copyright 2026. Andrew Wang.

Performance testing implementation.
*/
#include "perf_test.h"

#include <algorithm>
#include <fstream>
#include <print>
#include <random>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#include "benchmark.h"

using perf_test::show_comparison;
using std::default_random_engine;
using std::getline;
using std::ifstream;
using std::print;
using std::println;
using std::random_device;
using std::stoul;
using std::string;
using std::string_view;
using std::vector;

namespace ranges = std::ranges;
namespace views = std::views;

static constexpr auto SAMPLE_SIZE = 2'500UZ;
static constexpr auto ANNOUNCE_TEMPLATE = "{:<18}";

int main() {
  default_random_engine prng{random_device{}()};  // NOLINT(whitespace/braces)
  const auto words = perf_test::permute(perf_test::read_words(), prng);
  const auto solutions = perf_test::permute(perf_test::read_solutions(), prng);
  const auto sublist = perf_test::sample(words, SAMPLE_SIZE, prng);

  println("--- EXECUTING PERFORMANCE TESTS ---");

  set_perf set_benchmark;
  trie_perf trie_benchmark;

  print(ANNOUNCE_TEMPLATE, "Insert words:");
  show_comparison(set_benchmark.insert(words), trie_benchmark.insert(words));

  print(ANNOUNCE_TEMPLATE, "Count prefix:");
  show_comparison(set_benchmark.count(solutions),
                  trie_benchmark.count(solutions));

  print(ANNOUNCE_TEMPLATE, "Find prefix:");
  show_comparison(set_benchmark.find(solutions),
                  trie_benchmark.find(solutions));

  print(ANNOUNCE_TEMPLATE, "Contains words:");
  show_comparison(set_benchmark.contains(sublist),
                  trie_benchmark.contains(sublist));

  print(ANNOUNCE_TEMPLATE, "Forward iterate:");
  show_comparison(set_benchmark.forward_iterate(),
                  trie_benchmark.forward_iterate());

  print(ANNOUNCE_TEMPLATE, "Reverse iterate:");
  show_comparison(set_benchmark.reverse_iterate(),
                  trie_benchmark.reverse_iterate());

  print(ANNOUNCE_TEMPLATE, "Erase prefix:");
  show_comparison(set_benchmark.erase(solutions),
                  trie_benchmark.erase(solutions));

  println("--- COMPLETED PERFORMANCE TESTS ---");

  println("--- EXECUTING FINAL VERIFICATION ---");

  const auto& word_set = set_benchmark.peek();
  const auto& word_trie = trie_benchmark.peek();

  if (ranges::equal(word_set, word_trie)) {
    println("Forward ranges match");
  } else {
    println("Forward ranges do not match");
  }

  if (ranges::equal(ranges::reverse_view(word_set),
                    ranges::reverse_view(word_trie))) {
    println("Reverse ranges match");
  } else {
    println("Reverse ranges do not match");
  }

  println("--- COMPLETED FINAL VERIFICATION ---");
}

vector<string> perf_test::read_words() {
  ifstream fin{WORDS_FILE};
  if (!fin) throw perf_error("Could not open {}", WORDS_FILE);

  vector<string> words;
  words.reserve(WORDS_SIZE);
  for (string word; fin >> word;) {
    words.emplace_back(word);
  }

  if (WORDS_SIZE != words.size()) {
    throw perf_error("Expected {} words but got {}", WORDS_SIZE, words.size());
  }
  println("Imported {} words from {}", WORDS_SIZE, WORDS_FILE);
  return words;
}

vector<perf_test::solution_t> perf_test::read_solutions() {
  ifstream fin{SOLUTIONS_FILE};
  if (!fin) throw perf_error("Could not open {}", SOLUTIONS_FILE);

  string line;
  getline(fin, line);

  static constexpr auto expected_header = "prefix,begin,end,count";
  if (line != expected_header) {
    throw perf_error("Expected header to define columns {} but was {}",
                     expected_header, line);
  }

  vector<solution_t> solutions;
  solutions.reserve(SOLUTIONS_SIZE);
  while (getline(fin, line)) {
    const auto row = views::split(line, ',') | ranges::to<vector<string>>();
    if (row.size() != 4)
      throw perf_error("Expected rows of size 4 but was {}", row.size());
    solutions.emplace_back(row[0], row[1], row[2], stoul(row[3]));
  }

  if (SOLUTIONS_SIZE != solutions.size()) {
    throw perf_error("Expected {} solutions but got {}", SOLUTIONS_SIZE,
                     solutions.size());
  }
  println("Imported {} solutions from {}", SOLUTIONS_SIZE, SOLUTIONS_FILE);
  return solutions;
}

void perf_test::show_comparison(timeunit_t set_time, timeunit_t trie_time) {
  static constexpr auto COMPARE_TEMPLATE =
      "{:>4} was {:4.1f} x faster than {:>4}";
  const auto diff_ratio =
      static_cast<double>(max(set_time, trie_time).count()) /
      static_cast<double>(min(set_time, trie_time).count());
  if (set_time < trie_time) {
    println(COMPARE_TEMPLATE, "set", diff_ratio, "trie");
  } else {
    println(COMPARE_TEMPLATE, "trie", diff_ratio, "set");
  }
}
