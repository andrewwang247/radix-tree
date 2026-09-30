/*
Benchmark set performance.

Copyright 2026. Andrew Wang.
*/
#include "set_perf.h"

#include <iterator>
#include <ranges>
#include <span>
#include <string_view>

#include "lexi.h"
#include "perf_test.h"

using std::span;
using std::string_view;

namespace ranges = std::ranges;

namespace rt::perf_test {

ranges::range auto set_perf::prefix_range_for(string_view prefix) const {
  // Find the first item that's a prefix
  const auto begin = words.lower_bound(prefix);
  // Find where it stops being a prefix.
  const auto right_bound = lexicographic::increment(prefix);
  const auto end = words.lower_bound(right_bound);
  return ranges::subrange{begin, end};
}

timeunit_t set_perf::count(span<const solution_t> solutions) const {
  return count_impl(solutions, [this](string_view prf) {
    return ranges::distance(prefix_range_for(prf));
  });
}

timeunit_t set_perf::find(span<const solution_t> solutions) const {
  return find_impl(solutions,
                   [this](string_view prf) { return prefix_range_for(prf); });
}

timeunit_t set_perf::erase(span<const solution_t> solutions) {
  return erase_impl(solutions, [this](string_view prf) {
    const auto [begin, end] = prefix_range_for(prf);
    words.erase(begin, end);
  });
}

}  // namespace rt::perf_test
