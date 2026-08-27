/*
Copyright 2026. Andrew Wang.

Benchmarking class implementations.
*/
#include "benchmark.h"

#include <algorithm>
#include <iterator>
#include <limits>
#include <ranges>
#include <span>
#include <string>
#include <string_view>

#include "perf_test.h"

using std::numeric_limits;
using std::span;
using std::string;
using std::string_view;

namespace ranges = std::ranges;
namespace views = std::views;

string set_perf::lexicographic_increment(string_view word) {
  string owning_word{word};
  constexpr auto max_char = numeric_limits<char>::max();
  const auto last_non_max =
      ranges::find_if_not(owning_word | views::reverse,
                          [max_char](auto c) { return c == max_char; });
  // All characters are max char. Append min char.
  if (last_non_max == owning_word.rend()) {
    constexpr auto min_char = numeric_limits<char>::min();
    return owning_word + min_char;
  }
  // Increment the last non max char and remove everything after.
  ++*last_non_max;
  owning_word.erase(last_non_max.base(), owning_word.end());
  return owning_word;
}

ranges::range auto set_perf::prefix_range_for(string_view prefix) const {
  // Find the first item that's a prefix
  const auto begin = words.lower_bound(prefix);
  // Find where it stops being a prefix.
  const auto right_bound = lexicographic_increment(prefix);
  const auto end = words.lower_bound(right_bound);
  return ranges::subrange{begin, end};
}

timeunit_t set_perf::count(span<const perf_test::solution_t> solutions) const {
  return count_impl(solutions, [this](string_view prf) {
    return ranges::distance(prefix_range_for(prf));
  });
}

timeunit_t set_perf::find(span<const perf_test::solution_t> solutions) const {
  return find_impl(solutions,
                   [this](string_view prf) { return prefix_range_for(prf); });
}

timeunit_t set_perf::erase(span<const perf_test::solution_t> solutions) {
  return erase_impl(solutions, [this](string_view prf) {
    const auto [begin, end] = prefix_range_for(prf);
    words.erase(begin, end);
  });
}

timeunit_t trie_perf::count(span<const perf_test::solution_t> solutions) const {
  return count_impl(solutions,
                    [this](string_view prf) { return words.size(prf); });
}

timeunit_t trie_perf::find(span<const perf_test::solution_t> solutions) const {
  return find_impl(solutions, [this](string_view prf) {
    return ranges::subrange{words.begin(prf), words.end(prf)};
  });
}

timeunit_t trie_perf::erase(span<const perf_test::solution_t> solutions) {
  return erase_impl(solutions,
                    [this](string_view prf) { words.erase_prefix(prf); });
}
