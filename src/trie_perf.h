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
  timeunit_t count(std::span<const solution_t> solutions) const override;
  timeunit_t find(std::span<const solution_t> solutions) const override;
  timeunit_t contains(std::span<const solution_t> solutions) const override;
  timeunit_t erase(std::span<const solution_t> solutions) override;
};

}  // namespace rt::perf_test
