
/*
Benchmark trie performance.

Copyright 2026. Andrew Wang.
*/
#include "trie_perf.h"

#include <span>
#include <string_view>

#include "perf_test.h"

using std::span;
using std::string_view;

namespace rt::perf_test {

timeunit_t trie_perf::count_prefix(span<const solution_t> solutions) const {
  return count_impl(solutions,
                    [this](string_view prf) { return words.size(prf); });
}

timeunit_t trie_perf::find_prefix(span<const solution_t> solutions) const {
  return find_impl(solutions,
                   [this](string_view prf) { return words.find_prefix(prf); });
}

timeunit_t trie_perf::contains_prefix(span<const solution_t> solutions) const {
  return contains_impl(solutions, [this](string_view prf) {
    return words.contains_prefix(prf);
  });
}

timeunit_t trie_perf::erase_prefix(span<const solution_t> solutions) {
  return erase_impl(solutions,
                    [this](string_view prf) { words.erase_prefix(prf); });
}

}  // namespace rt::perf_test
