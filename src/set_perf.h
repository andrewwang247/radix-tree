/*
Benchmark set performance.

Copyright 2026. Andrew Wang.
*/
#pragma once

#include <functional>
#include <ranges>
#include <set>
#include <span>
#include <string>
#include <string_view>

#include "benchmark.h"

namespace rt::perf_test {

/**
 * @brief Perf class template for std::set.
 */
class set_perf final : public perf<std::set<std::string, std::less<>>> {
 private:
  /**
   * @brief Locate boundaries of a prefix range.
   * @param prefix The prefix to locate.
   * @return The range of words with prefix.
   */
  std::ranges::input_range auto prefix_range_for(std::string_view prefix) const;

 public:
  timeunit_t count(std::span<const solution_t> solutions) const override;
  timeunit_t find(std::span<const solution_t> solutions) const override;
  timeunit_t contains(std::span<const solution_t> solutions) const override;
  timeunit_t erase(std::span<const solution_t> solutions) override;
};

}  // namespace rt::perf_test
