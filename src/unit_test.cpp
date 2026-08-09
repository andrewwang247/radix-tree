/*
Copyright 2026. Andrew Wang.

Unit testing implementation.
*/
#include "unit_test.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <random>
#include <ranges>
#include <utility>

#include "trie.h"

using std::cout;
using std::default_random_engine;
using std::ios_base;
using std::random_device;

namespace ranges = std::ranges;

int main() {
  ios_base::sync_with_stdio(false);

  cout << "--- EXECUTING UNIT TESTS ---\n";
  assert(ranges::is_sorted(unit_test::SORTED_WORDS));
  unit_test::empty();
  unit_test::single();
  unit_test::find();
  unit_test::insert();
  unit_test::erase();
  unit_test::forward_iterate();
  unit_test::reverse_iterate();
  unit_test::copy_move();
  unit_test::comparison();
  unit_test::arithmetic();
  unit_test::representation();
  cout << "--- FINISHED UNIT TESTS ---\n";
}

trie unit_test::get_trie() {
  static random_device rdev;
  static default_random_engine rng{rdev()};
  auto copy{SORTED_WORDS};
  ranges::shuffle(copy, rng);
  return trie{copy};
}

void unit_test::empty() {
  cout << "Empty test";
  trie tr;
  assert(tr.empty());
  assert(tr.empty("hello"));
  assert(tr.size() == 0);
  assert(tr.size("world") == 0);

  assert(!tr.begin());
  assert(!tr.end());
  assert(tr.begin() == tr.end());
  assert(tr.find("test") == tr.end());
  assert(tr.find_prefix("test") == tr.end());
  assert(tr.find("") == tr.end());
  assert(tr.find_prefix("") == tr.end());

  tr.insert("");
  assert(!tr.empty());
  assert(tr.empty("hello"));
  assert(tr.size() == 1);
  assert(tr.size("world") == 0);

  assert(tr.begin());
  assert(!tr.end());
  assert(tr.begin()->empty());
  assert(tr.find("test") == tr.end());
  assert(tr.find_prefix("test") == tr.end());
  assert(tr.find("")->empty());
  assert(tr.find_prefix("")->empty());
  cout << " passed\n";
}

void unit_test::single() {
  cout << "Singleton test";
  trie tr;
  tr.insert("single");
  assert(!tr.empty());
  assert(tr.empty("hello"));
  assert(!tr.empty("sin"));
  assert(tr.size() == 1);
  assert(tr.size("world") == 0);
  assert(tr.size("si") == 1);

  assert(tr.begin());
  assert(!tr.end());
  assert(*tr.begin() == "single");
  assert(tr.find("test") == tr.end());
  assert(tr.find_prefix("test") == tr.end());
  assert(tr.find("") == tr.end());
  assert(*tr.find_prefix("") == "single");
  assert(tr.find("sin") == tr.end());
  assert(*tr.find_prefix("sin") == "single");
  assert(*tr.find("single") == "single");
  assert(*tr.find_prefix("single") == "single");
  cout << " passed\n";
}

void unit_test::find() {
  cout << "Find test";
  const auto tr = get_trie();

  assert(!tr.empty());
  assert(tr.size() == 13);
  assert(tr.size("ma") == 7);

  const auto exact_iter = tr.find("corn");
  assert(exact_iter != tr.end());
  assert(*exact_iter == "corn");

  const auto prf_iter = tr.find_prefix("mate");
  assert(prf_iter != tr.end());
  assert(*prf_iter == "material");

  const auto exact_prf_iter = tr.find_prefix("contaminate");
  assert(exact_prf_iter != tr.end());
  assert(*exact_prf_iter == "contaminate");

  const auto missing_exact_iter = tr.find("testing");
  assert(missing_exact_iter == tr.end());

  const auto missing_prf_iter = tr.find("conk");
  assert(missing_prf_iter == tr.end());
  cout << " passed\n";
}

void unit_test::insert() {
  cout << "Insert test";
  trie tr;

  auto iter = tr.insert("math");
  assert(iter != tr.end());
  assert(*iter == "math");
  assert(tr.size("math") == 1);
  assert(!tr.empty("mat"));

  iter = tr.insert("malleable");
  assert(iter != tr.end());
  assert(*iter == "malleable");
  assert(tr.size() == 2);
  assert(!tr.empty("ma"));

  tr.insert("regression");  // ensure idempotence
  iter = tr.insert("regression");
  assert(iter != tr.end());
  assert(*iter == "regression");
  assert(tr.size("m") == 2);
  assert(tr.size() == 3);
  assert(!tr.empty("reg"));
  cout << " passed\n";
}

void unit_test::erase() {
  cout << "Erase test";
  auto tr = get_trie();

  // Erase something that does not exist.
  tr.erase_prefix("random");
  tr.erase("cplusplus");
  assert(tr.size() == 13);

  // Erase a leaf node.
  tr.erase("maternal");  // ensure idempotence
  tr.erase("maternal");
  assert(tr.size() == 12);
  assert(!tr.empty());
  assert(tr.find("maternal") == tr.end());
  assert(tr.size("mat") == 4);
  assert(tr.empty("matern"));

  // Erase non-degenerate internal node.
  tr.erase("mat");  // ensure idempotence
  tr.erase("mat");
  auto iter = tr.find_prefix("mat");
  assert(iter != tr.end());
  assert(*iter == "material");
  assert(tr.size("ma") == 5);
  assert(!tr.empty("mat"));

  // Erase degenerate internal node.
  tr.erase("corn");  // ensure idempotence
  tr.erase("corn");
  iter = tr.find("corner");
  assert(iter != tr.end());
  assert(*iter == "corner");
  assert(tr.size("co") == 5);

  tr.erase_prefix("con");  // ensure idempotence
  tr.erase_prefix("con");
  assert(tr.find("contain") == tr.end());
  assert(tr.find("contaminate") == tr.end());
  assert(tr.find_prefix("con") == tr.end());

  // Try clearing.
  tr.clear();
  assert(tr.empty());
  assert(tr.size() == 0);
  cout << " passed\n";
}

void unit_test::forward_iterate() {
  cout << "Forward iteration test";
  const auto tr = get_trie();

  // Full range iteration
  assert(ranges::equal(SORTED_WORDS, tr));

  // Only iterate over subportion.
  const auto find_compute = ranges::find(SORTED_WORDS, "compute");
  const auto find_corner = ranges::find(SORTED_WORDS, "corner");
  assert(ranges::equal(ranges::subrange{find_compute, next(find_corner)},
                       ranges::subrange{tr.begin("co"), tr.end("co")}));

  const auto find_mahjong = ranges::find(SORTED_WORDS, "mahjong");
  const auto find_matrix = ranges::find(SORTED_WORDS, "matrix");
  assert(ranges::equal(ranges::subrange{find_mahjong, next(find_matrix)},
                       ranges::subrange{tr.begin("ma"), tr.end("ma")}));

  // Singular word range.
  const auto single_start = tr.begin("contaminate");
  const auto single_finish = tr.end("contaminate");
  assert(single_start != tr.end());
  assert(*single_start == "contaminate");
  assert(single_finish != tr.end());
  assert(*single_finish == "corn");

  // Non-existant range.
  assert(tr.begin("cops") == tr.end());
  assert(*tr.end("cops") == "corn");
  assert(!tr.end());
  cout << " passed\n";
}

void unit_test::reverse_iterate() {
  cout << "Reverse iteration test";
  const auto tr = get_trie();

  const auto backwards = ranges::reverse_view(SORTED_WORDS);
  // Full range iteration
  assert(ranges::equal(backwards, ranges::reverse_view(tr)));

  // Only iterate over subportion.
  const auto find_corner = ranges::find(backwards, "corner");
  const auto find_compute = ranges::find(backwards, "compute");
  assert(ranges::equal(
      ranges::subrange{find_corner, next(find_compute)},
      ranges::reverse_view(ranges::subrange{tr.begin("co"), tr.end("co")})));

  const auto find_matrix = ranges::find(backwards, "matrix");
  const auto find_mahjong = ranges::find(backwards, "mahjong");
  assert(ranges::equal(
      ranges::subrange{find_matrix, next(find_mahjong)},
      ranges::reverse_view(ranges::subrange{tr.begin("ma"), tr.end("ma")})));

  cout << " passed\n";
}

void unit_test::copy_move() {
  cout << "Copy and Move test";
  auto original = get_trie();

  trie copied(original);
  assert(ranges::equal(original, copied));

  copied.clear();
  copied = original;
  assert(ranges::equal(original, copied));

  trie moved{std::move(original)};
  assert(ranges::equal(SORTED_WORDS, moved));

  copied.clear();
  moved = std::move(copied);
  assert(ranges::empty(moved));

  cout << " passed\n";
}

void unit_test::comparison() {
  cout << "Comparison test";
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
  cout << " passed\n";
}

void unit_test::arithmetic() {
  cout << "Arithmetic test";

  const trie tr{"mahogany", "mahjong",     "compute", "computer", "matrix",
                "math",     "contaminate", "corn",    "corner",   "material",
                "mat",      "maternal",    "contain"};
  const trie t1{"compute", "contain",  "corn",  "mahjong",
                "mat",     "maternal", "matrix"};
  const trie t2{"computer", "contaminate", "corner",
                "mahogany", "material",    "math"};
  const trie ex{"some", "extra", "stuff"};

  assert(t1 + t2 == tr);
  assert(tr - t2 == t1);
  assert(tr - t1 == t2);
  assert((tr - t1 - t2).empty());

  assert(tr - ex == tr);
  assert(tr < tr + ex);
  cout << " passed\n";
}

void unit_test::representation() {
  cout << "Representation test";
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
  assert(mat_iter);
  assert(*mat_iter == "mat");
  constexpr auto MAT_JSON = R"({"er":{"ial":{},"nal":{}},"h":{},"rix":{}})";
  assert(mat_iter.to_json(false) == MAT_JSON);

  cout << " passed\n";
}
