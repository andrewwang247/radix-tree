/*
Copyright 2026. Andrew Wang.

Unit testing implementation.
*/
#include "unit_test.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "trie.h"

using std::copy;
using std::cout;
using std::string;
using std::vector;

vector<string> extract_range(const Trie::iterator& start,
                             const Trie::iterator& finish) {
  vector<string> range;
  copy(start, finish, back_inserter(range));
  return range;
}

bool unit_test::empty() {
  cout << "Empty test";
  Trie tr;
  if (!tr.empty()) return false;
  if (!tr.empty("hello")) return false;
  if (tr.size() != 0) return false;
  if (tr.size("world") != 0) return false;
  if (tr.find("test") != tr.end()) return false;
  return true;
}

bool unit_test::find() {
  cout << "Find test";
  Trie tr{"mahogany", "mahjong",     "compute", "computer", "matrix",
          "math",     "contaminate", "corn",    "corner",   "material",
          "mat",      "maternal",    "contain"};

  if (tr.empty()) return false;
  if (tr.size() != 13) return false;
  if (tr.size("ma") != 7) return false;

  const auto exact_iter = tr.find("corn");
  if (exact_iter == tr.end() || *exact_iter != "corn") return false;

  const auto prf_iter = tr.find_prefix("mate");
  if (prf_iter == tr.end() || *prf_iter != "material") return false;

  const auto exact_prf_iter = tr.find_prefix("contaminate");
  if (exact_prf_iter == tr.end() || *exact_prf_iter != "contaminate")
    return false;

  const auto missing_exact_iter = tr.find("testing");
  if (missing_exact_iter != tr.end()) return false;

  const auto missing_prf_iter = tr.find("conk");
  if (missing_prf_iter != tr.end()) return false;

  return true;
}

bool unit_test::insert() {
  cout << "Insert test";
  Trie tr;

  auto iter = tr.insert("math");
  if (iter == tr.end() || *iter != "math") return false;
  if (tr.size("math") != 1 || tr.empty()) return false;

  iter = tr.insert("malleable");
  if (iter == tr.end() || *iter != "malleable") return false;
  if (tr.size() != 2 || tr.empty()) return false;

  iter = tr.insert("regression");
  if (iter == tr.end() || *iter != "regression") return false;
  if (tr.size("m") != 2) return false;
  if (tr.size() != 3) return false;

  return true;
}

bool unit_test::erase() {
  cout << "Erase test";
  Trie tr{"mahogany", "mahjong",     "compute", "computer", "matrix",
          "math",     "contaminate", "corn",    "corner",   "material",
          "mat",      "maternal",    "contain"};

  // Erase something that does not exist.
  tr.erase_prefix("random");
  tr.erase("cplusplus");
  if (tr.size() != 13) return false;

  // Erase a leaf node.
  tr.erase("maternal");
  if (tr.size() != 12 || tr.empty()) return false;
  if (tr.find("maternal") != tr.end()) return false;
  if (tr.size("mat") != 4 || !tr.empty("matern")) return false;

  // Erase non-degenerate internal node.
  tr.erase("mat");
  auto iter = tr.find_prefix("mat");
  if (iter == tr.end() || *iter != "material") return false;
  if (tr.size("ma") != 5 || tr.empty("mat")) return false;

  // Erase degenerate internal node.
  tr.erase("corn");
  iter = tr.find("corner");
  if (iter == tr.end() || *iter != "corner") return false;
  if (tr.size("co") != 5) return false;

  tr.erase_prefix("con");
  if (tr.find("contain") != tr.end()) return false;
  if (tr.find("contaminate") != tr.end()) return false;
  if (tr.find_prefix("con") != tr.end()) return false;
  if (tr.size("co") != 3) return false;

  // Try clearing.
  tr.clear();
  if (!tr.empty()) return false;
  if (tr.size() != 0) return false;

  return true;
}

bool unit_test::iterate() {
  cout << "Iteration test";

  vector<string> words{"compute", "computer", "contain",  "contaminate",
                       "corn",    "corner",   "mahjong",  "mahogany",
                       "mat",     "material", "maternal", "math",
                       "matrix"};

  // Also tests input iterator constructor.
  const Trie tr(words.begin(), words.end());

  const auto total_iterated = extract_range(tr.begin(), tr.end());

  if (words != total_iterated) return false;

  // Only iterate over subportion.
  vector<string> co_words{"compute",     "computer", "contain",
                          "contaminate", "corn",     "corner"};
  const auto co_iterated = extract_range(tr.begin("co"), tr.end("co"));
  if (co_words != co_iterated) return false;

  vector<string> ma_words{"mahjong",  "mahogany", "mat",   "material",
                          "maternal", "math",     "matrix"};
  const auto ma_iterated = extract_range(tr.begin("ma"), tr.end("ma"));
  if (ma_words != ma_iterated) return false;

  // Prefix subportion.
  vector<string> mate_words{"material", "maternal"};
  const auto mate_iterated = extract_range(tr.begin("mate"), tr.end("mate"));
  if (mate_words != mate_iterated) return false;

  // Singular word range.
  const auto single_start = tr.begin("contaminate");
  const auto single_finish = tr.end("contaminate");
  if (single_start == tr.end() || *single_start != "contaminate") return false;
  if (single_finish == tr.end() || *single_finish != "corn") return false;

  // Non-existant range.
  if (tr.begin("cops") != tr.end()) return false;
  if (*tr.end("cops") != "corn") return false;

  return true;
}

bool unit_test::copy_move() {
  cout << "Copy and Move test";

  Trie original{"mahogany", "mahjong",     "compute", "computer", "matrix",
                "math",     "contaminate", "corn",    "corner",   "material",
                "mat",      "maternal",    "contain"};
  const auto orig_vec = extract_range(original.begin(), original.end());

  Trie copied(original);
  const auto copied_vec = extract_range(copied.begin(), copied.end());
  if (orig_vec != copied_vec) return false;

  Trie moved{std::move(original)};
  const auto moved_vec = extract_range(moved.begin(), moved.end());
  return orig_vec == moved_vec;
}

bool unit_test::comparison() {
  cout << "Comparison test";

  Trie t1{"mahogany", "mahjong",     "compute", "computer", "matrix",
          "math",     "contaminate", "corn",    "corner",   "material",
          "mat",      "maternal",    "contain"};

  Trie t2{"mahogany", "mahjong",     "compute", "computer", "matrix",
          "math",     "contaminate", "corn",    "corner",   "material",
          "mat",      "maternal",    "contain"};

  // Test equality
  if (t1 != t2) return false;

  // Test inequality
  t1.erase("material");
  return t1 < t2;
}

bool unit_test::arithmetic() {
  cout << "Arithmetic test";

  const Trie tr{"mahogany", "mahjong",     "compute", "computer", "matrix",
                "math",     "contaminate", "corn",    "corner",   "material",
                "mat",      "maternal",    "contain"};
  const Trie t1{"compute", "contain",  "corn",  "mahjong",
                "mat",     "maternal", "matrix"};
  const Trie t2{"computer", "contaminate", "corner",
                "mahogany", "material",    "math"};
  const Trie ex{"some", "extra", "stuff"};

  if (t1 + t2 != tr) return false;
  if (tr - t2 != t1) return false;
  if (tr - t1 != t2) return false;
  if (!(tr - t1 - t2).empty()) return false;

  if (tr - ex != tr) return false;
  if (tr >= tr + ex) return false;

  return true;
}
