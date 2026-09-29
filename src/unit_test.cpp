/*
Unit testing trie functionality.

Copyright 2026. Andrew Wang.
*/
#include "unit_test.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <compare>
#include <concepts>
#include <iterator>
#include <print>
#include <random>
#include <ranges>
#include <string_view>
#include <type_traits>
#include <utility>

#include "trie.h"

using std::default_random_engine;
using std::println;
using std::random_device;
using std::string_view;
using std::to_array;

namespace ranges = std::ranges;
namespace views = std::views;

int main() {
  println("--- EXECUTING UNIT TESTS ---");
  rt::unit_test::concepts();
  rt::unit_test::empty();
  rt::unit_test::single();
  rt::unit_test::find();
  rt::unit_test::insert();
  rt::unit_test::erase();
  rt::unit_test::forward_iterate();
  rt::unit_test::reverse_iterate();
  rt::unit_test::copy_move();
  rt::unit_test::comparison();
  rt::unit_test::arithmetic();
  rt::unit_test::representation();
  println("--- COMPLETED UNIT TESTS ---");
}

namespace rt {

namespace unit_test {

static constexpr auto RESULT_TEMPLATE = "Test {:<20} passed";

// NOLINTBEGIN
static constexpr auto SORTED_WORDS = to_array<string_view>(
    {"compute", "computer", "contain", "contaminate", "corn", "corner",
     "mahjong", "mahogany", "mat", "material", "maternal", "math", "matrix"});
// NOLINTEND

static_assert(ranges::is_sorted(SORTED_WORDS),
              "Unit tests assume provided words are sorted");

}  // namespace unit_test

trie unit_test::get_trie() {
  static auto prng =
      default_random_engine{random_device{}()};  // NOLINT(whitespace/braces)
  auto copy = SORTED_WORDS;
  ranges::shuffle(copy, prng);
  trie tr{copy};
  tr.assert_invariants();
  return tr;
}

void unit_test::concepts() {
  static_assert(std::regular<trie>);
  static_assert(std::three_way_comparable<const trie>);
  static_assert(std::totally_ordered<const trie>);
  static_assert(std::is_nothrow_destructible_v<const trie>);
  static_assert(std::is_nothrow_move_constructible_v<trie>);

  static_assert(ranges::bidirectional_range<const trie>);
  static_assert(ranges::common_range<const trie>);
  static_assert(!ranges::sized_range<const trie>);  // not O(1)
  static_assert(ranges::viewable_range<trie>);

  println(RESULT_TEMPLATE, "concepts");
}

void unit_test::empty() {
  trie tr;
  tr.assert_invariants();

  assert(tr.empty());
  assert(tr.empty("hello"));
  assert(tr.size() == 0);
  assert(tr.size("world") == 0);

  assert(!tr.begin());
  assert(!tr.end());
  assert(tr.begin() == tr.end());
  assert(!tr.contains(""));
  assert(tr.find_prefix("") == tr.end());

  tr.insert("");
  tr.assert_invariants();

  assert(!tr.empty());
  assert(tr.empty("hello"));
  assert(tr.size() == 1);
  assert(tr.size("world") == 0);

  assert(tr.begin());
  assert(!tr.end());
  assert(tr.begin()->empty());
  assert(!tr.contains("test"));
  assert(tr.find_prefix("test") == tr.end());
  assert(tr.find("")->empty());
  assert(tr.find_prefix("")->empty());

  println(RESULT_TEMPLATE, "empty");
}

void unit_test::single() {
  trie tr;
  tr.insert("single");
  tr.assert_invariants();

  assert(!tr.empty());
  assert(tr.empty("hello"));
  assert(!tr.empty("sin"));
  assert(tr.size() == 1);
  assert(tr.size("world") == 0);
  assert(tr.size("si") == 1);

  assert(tr.begin());
  assert(!tr.end());
  assert(*tr.begin() == "single");
  assert(!tr.contains("test"));
  assert(tr.find("test") == tr.end());
  assert(tr.find_prefix("test") == tr.end());
  assert(!tr.contains(""));
  assert(tr.find("") == tr.end());
  assert(*tr.find_prefix("") == "single");
  assert(!tr.contains("sin"));
  assert(tr.find("sin") == tr.end());
  assert(*tr.find_prefix("sin") == "single");
  assert(tr.contains("single"));
  assert(*tr.find("single") == "single");
  assert(*tr.find_prefix("single") == "single");

  println(RESULT_TEMPLATE, "singleton");
}

void unit_test::find() {
  const auto tr = get_trie();

  assert(!tr.empty());
  assert(tr.size() == 13);
  assert(tr.size("ma") == 7);
  assert(!tr.contains("co"));
  assert(tr.contains("mat"));

  assert(tr.contains("corn"));
  const auto exact_iter = tr.find("corn");
  assert(exact_iter != tr.end());
  assert(*exact_iter == "corn");

  assert(!tr.contains("mate"));
  const auto prf_iter = tr.find_prefix("mate");
  assert(prf_iter != tr.end());
  assert(*prf_iter == "material");

  assert(tr.contains("contaminate"));
  const auto exact_prf_iter = tr.find_prefix("contaminate");
  assert(exact_prf_iter != tr.end());
  assert(*exact_prf_iter == "contaminate");

  assert(!tr.contains("testing"));
  const auto missing_exact_iter = tr.find("testing");
  assert(missing_exact_iter == tr.end());

  assert(!tr.contains("conk"));
  const auto missing_prf_iter = tr.find("conk");
  assert(missing_prf_iter == tr.end());

  println(RESULT_TEMPLATE, "find");
}

void unit_test::insert() {
  trie tr;

  assert(!tr.contains("math"));
  auto iter = tr.insert("math");
  tr.assert_invariants();

  assert(tr.contains("math"));
  assert(iter != tr.end());
  assert(*iter == "math");
  assert(tr.size("math") == 1);
  assert(!tr.empty("mat"));

  assert(!tr.contains("malleable"));
  iter = tr.insert("malleable");
  tr.assert_invariants();

  assert(tr.contains("malleable"));
  assert(iter != tr.end());
  assert(*iter == "malleable");
  assert(tr.size() == 2);
  assert(!tr.empty("ma"));

  assert(!tr.contains("regression"));
  tr.assert_invariants();
  tr.insert("regression");  // ensure idempotence
  tr.assert_invariants();

  assert(tr.contains("regression"));
  iter = tr.insert("regression");
  tr.assert_invariants();

  assert(iter != tr.end());
  assert(*iter == "regression");
  assert(tr.size("m") == 2);
  assert(tr.size() == 3);
  assert(!tr.empty("reg"));

  println(RESULT_TEMPLATE, "insert");
}

void unit_test::erase() {
  auto tr = get_trie();

  // Erase something that does not exist.
  tr.erase_prefix("random");
  tr.assert_invariants();
  tr.erase("cplusplus");
  tr.assert_invariants();
  assert(tr.size() == 13);

  // Erase a leaf node.
  tr.erase("maternal");  // ensure idempotence
  tr.assert_invariants();
  tr.erase("maternal");
  tr.assert_invariants();

  assert(!tr.contains("maternal"));
  assert(tr.size() == 12);
  assert(!tr.empty());
  assert(tr.find("maternal") == tr.end());
  assert(tr.size("mat") == 4);
  assert(tr.empty("matern"));

  // Erase non-degenerate internal node.
  tr.erase("mat");  // ensure idempotence
  tr.assert_invariants();
  tr.erase("mat");
  tr.assert_invariants();

  assert(!tr.contains("mat"));
  auto iter = tr.find_prefix("mat");
  assert(iter != tr.end());
  assert(*iter == "material");
  assert(tr.size("ma") == 5);
  assert(!tr.empty("mat"));

  // Erase degenerate internal node.
  tr.erase("corn");  // ensure idempotence
  tr.assert_invariants();
  tr.erase("corn");
  tr.assert_invariants();

  assert(tr.contains("corner"));
  iter = tr.find("corner");
  assert(iter != tr.end());
  assert(*iter == "corner");
  assert(tr.size("co") == 5);

  tr.erase_prefix("con");  // ensure idempotence
  tr.assert_invariants();
  tr.erase_prefix("con");
  tr.assert_invariants();

  assert(!tr.contains("contain"));
  assert(!tr.contains("contaminate"));
  assert(tr.find("contain") == tr.end());
  assert(tr.find("contaminate") == tr.end());
  assert(tr.find_prefix("con") == tr.end());

  // Try clearing.
  tr.clear();
  tr.assert_invariants();
  assert(tr.empty());
  assert(tr.size() == 0);

  println(RESULT_TEMPLATE, "erase");
}

void unit_test::forward_iterate() {
  const auto tr = get_trie();

  // Full range iteration
  assert(ranges::equal(SORTED_WORDS, tr));

  // Only iterate over subportion.
  const auto* compute = ranges::find(SORTED_WORDS, "compute");
  const auto* corner = ranges::find(SORTED_WORDS, "corner");
  assert(ranges::equal(ranges::subrange{compute, std::next(corner)},
                       tr.subrange("co")));

  const auto* mahjong = ranges::find(SORTED_WORDS, "mahjong");
  const auto* matrix = ranges::find(SORTED_WORDS, "matrix");
  assert(ranges::equal(ranges::subrange{mahjong, std::next(matrix)},
                       tr.subrange("ma")));

  // Singular word range.
  const auto [ctm_begin, ctm_end] = tr.subrange("contaminate");
  assert(ctm_begin != tr.end());
  assert(*ctm_begin == "contaminate");
  assert(ctm_end != tr.end());
  assert(*ctm_end == "corn");

  // Non-existant range.
  assert(tr.begin("cops") == tr.end());
  assert(*tr.end("cops") == "corn");
  assert(!tr.end());

  println(RESULT_TEMPLATE, "forward iterate");
}

void unit_test::reverse_iterate() {
  const auto tr = get_trie();

  constexpr auto backwards = SORTED_WORDS | views::reverse;
  // Full range iteration
  assert(ranges::equal(backwards, tr | views::reverse));

  // Only iterate over subportion.
  const auto corner = ranges::find(backwards, "corner");
  const auto compute = ranges::find(backwards, "compute");
  assert(ranges::equal(ranges::subrange{corner, std::next(compute)},
                       tr.subrange("co") | views::reverse));

  const auto matrix = ranges::find(backwards, "matrix");
  const auto mahjong = ranges::find(backwards, "mahjong");
  assert(ranges::equal(ranges::subrange{matrix, std::next(mahjong)},
                       tr.subrange("ma") | views::reverse));

  println(RESULT_TEMPLATE, "reverse iterate");
}

void unit_test::copy_move() {
  auto original = get_trie();

  trie copied(original);
  copied.assert_invariants();
  assert(ranges::equal(original, copied));

  copied.clear();
  copied = original;
  copied.assert_invariants();
  assert(ranges::equal(original, copied));

  trie moved{std::move(original)};
  moved.assert_invariants();
  assert(ranges::equal(SORTED_WORDS, moved));

  copied.clear();
  moved = std::move(copied);
  moved.assert_invariants();
  assert(moved.empty());

  println(RESULT_TEMPLATE, "copy and move");
}

void unit_test::comparison() {
  auto t1 = get_trie();
  const auto t2 = get_trie();

  // Test equality
  assert(t1 == t2);
  assert(!(t1 != t2));
  // Test inequality
  t1.erase("material");
  assert(t1 < t2);
  assert(t2 > t1);
  assert(t1 <= t2);
  assert(t2 >= t1);

  println(RESULT_TEMPLATE, "comparison");
}

void unit_test::arithmetic() {
  const trie tr{"mahogany", "mahjong",     "compute", "computer", "matrix",
                "math",     "contaminate", "corn",    "corner",   "material",
                "mat",      "maternal",    "contain"};
  tr.assert_invariants();

  const trie t1{"compute", "contain",  "corn",  "mahjong",
                "mat",     "maternal", "matrix"};
  t1.assert_invariants();

  const trie t2{"computer", "contaminate", "corner",
                "mahogany", "material",    "math"};
  t2.assert_invariants();

  const trie ex{"some", "extra", "stuff"};
  ex.assert_invariants();

  assert(t1 + t2 == tr);
  assert(tr - t2 == t1);
  assert(tr - t1 == t2);
  assert((tr - t1 - t2).empty());

  assert(tr - ex == tr);
  assert(tr < tr + ex);

  println(RESULT_TEMPLATE, "arithmetic");
}

void unit_test::representation() {
  const auto tr = get_trie();

  assert(tr.end().to_json(true) == "{}");

  constexpr auto TR_JSON = R"({"co":{"mpute":{"r":{}},"nta":{"in":{},)"
                           R"("minate":{}},"rn":{"er":{}}},"ma":{"h":)"
                           R"({"jong":{},"ogany":{}},"t":{"er":{"ial":)"
                           R"({},"nal":{}},"h":{},"rix":{}}}})";
  assert(tr.to_json() == TR_JSON);

  const auto com_prf = tr.find_prefix("com");
  assert(*com_prf == "compute");
  constexpr auto COM_JSON = R"({"end":true,"children":)"
                            R"({"r":{"end":true,"children":)"
                            "{}}}}";
  assert(com_prf.to_json(true) == COM_JSON);

  const auto mat_iter = tr.find("mat");
  assert(*mat_iter == "mat");
  constexpr auto MAT_JSON = R"({"er":{"ial":{},"nal":{}},"h":{},"rix":{}})";
  assert(mat_iter.to_json(false) == MAT_JSON);
  constexpr auto MAT_JSON_ENDS = R"({"end":true,"children":{"er":)"
                                 R"({"end":false,"children":{"ial":)"
                                 R"({"end":true,"children":{}},"nal":)"
                                 R"({"end":true,"children":{}}}},"h":)"
                                 R"({"end":true,"children":{}},"rix":)"
                                 R"({"end":true,"children":{}}}})";
  assert(mat_iter.to_json(true) == MAT_JSON_ENDS);

  println(RESULT_TEMPLATE, "representation");
}

}  // namespace rt
