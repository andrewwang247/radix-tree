/*
Copyright 2026. Andrew Wang.

Benchmarking class implementations.
*/
#include "benchmark.h"

#include <algorithm>
#include <iterator>
#include <limits>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#include "perf_test.h"

using std::numeric_limits;
using std::string;
using std::string_view;
using std::vector;

namespace ranges = std::ranges;
namespace views = std::views;

string set_perf::lexicographic_increment(string word) {
  constexpr auto max_char = numeric_limits<decltype(word)::value_type>::max();
  const auto last_non_max = ranges::find_if_not(
      word | views::reverse, [max_char](auto c) { return c == max_char; });
  // All characters are max char. Append min char.
  if (last_non_max == word.rend()) {
    constexpr auto min_char = numeric_limits<decltype(word)::value_type>::min();
    return word + min_char;
  }
  // Increment the last non max char and remove everything after.
  ++*last_non_max;
  word.erase(last_non_max.base(), word.end());
  return word;
}

ranges::range auto set_perf::prefix_range_for(string_view prefix) const {
  // Find the first item that's a prefix
  const auto begin = words.lower_bound(prefix);
  // Find where it stops being a prefix.
  const auto right_bound = lexicographic_increment(string{prefix});
  const auto end = words.lower_bound(right_bound);
  return ranges::subrange{begin, end};
}

timeunit_t set_perf::count(
    const vector<perf_test::solution_t>& solutions) const {
  return count_impl(solutions, [this](string_view prf) {
    return ranges::distance(prefix_range_for(prf));
  });
}

timeunit_t set_perf::find(
    const vector<perf_test::solution_t>& solutions) const {
  return find_impl(solutions,
                   [this](string_view prf) { return prefix_range_for(prf); });
}

timeunit_t set_perf::erase(const vector<perf_test::solution_t>& solutions) {
  return erase_impl(solutions, [this](string_view prf) {
    const auto [begin, end] = prefix_range_for(prf);
    words.erase(begin, end);
  });
}

timeunit_t trie_perf::count(
    const vector<perf_test::solution_t>& solutions) const {
  return count_impl(solutions,
                    [this](string_view prf) { return words.size(prf); });
}

timeunit_t trie_perf::find(
    const vector<perf_test::solution_t>& solutions) const {
  return find_impl(solutions, [this](string_view prf) {
    return ranges::subrange{words.begin(prf), words.end(prf)};
  });
}

timeunit_t trie_perf::erase(const vector<perf_test::solution_t>& solutions) {
  return erase_impl(solutions,
                    [this](string_view prf) { words.erase_prefix(prf); });
}
