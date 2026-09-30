/*
Benchmark trie performance.

Copyright 2026. Andrew Wang.
*/
#pragma once

#include <span>

#include "benchmark.h"

namespace rt::perf_test {

/**
 * @brief Perf class template for trie.
 */
class trie_perf final : public perf<trie> {
 public:
  timeunit_t count_prefix(std::span<const solution_t> solutions) const override;
  timeunit_t find_prefix(std::span<const solution_t> solutions) const override;
  timeunit_t contains_prefix(
      std::span<const solution_t> solutions) const override;
  timeunit_t erase_prefix(std::span<const solution_t> solutions) override;
};

}  // namespace rt::perf_test
