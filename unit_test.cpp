/*
Copyright 2026. Andrew Wang.

Unit testing implementation.
*/
#include "unit_test.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "trie.h"

using std::copy;
using std::cout;
using std::string;
using std::vector;

vector<string> extract_range(const iterator& start, const iterator& finish) {
  vector<string> range;
  copy(start, finish, back_inserter(range));
  return range;
}

void unit_test::empty_single() {
  cout << "Empty and Singleton test";
  {
    trie tr;
    assert(tr.empty());
    assert(tr.empty("hello"));
    assert(tr.size() == 0);
    assert(tr.size("world") == 0);

    assert(tr.begin() == tr.end());
    assert(tr.find("test") == tr.end());
    assert(tr.find_prefix("test") == tr.end());
    assert(tr.find("") == tr.end());
    assert(tr.find_prefix("") == tr.end());
  }
  {
    trie tr;
    tr.insert("");
    assert(!tr.empty());
    assert(tr.empty("hello"));
    assert(tr.size() == 1);
    assert(tr.size("world") == 0);

    assert(*tr.begin() == "");
    assert(tr.find("test") == tr.end());
    assert(tr.find_prefix("test") == tr.end());
    assert(*tr.find("") == "");
    assert(*tr.find_prefix("") == "");
  }
  {
    trie tr;
    tr.insert("single");
    assert(!tr.empty());
    assert(tr.empty("hello"));
    assert(!tr.empty("sin"));
    assert(tr.size() == 1);
    assert(tr.size("world") == 0);
    assert(tr.size("si") == 1);

    assert(*tr.begin() == "single");
    assert(tr.find("test") == tr.end());
    assert(tr.find_prefix("test") == tr.end());
    assert(tr.find("") == tr.end());
    assert(*tr.find_prefix("") == "single");
    assert(tr.find("sin") == tr.end());
    assert(*tr.find_prefix("sin") == "single");
    assert(*tr.find("single") == "single");
    assert(*tr.find_prefix("single") == "single");
  }
  cout << " passed\n";
}

void unit_test::find() {
  cout << "Find test";
  trie tr{"mahogany", "mahjong",     "compute", "computer", "matrix",
          "math",     "contaminate", "corn",    "corner",   "material",
          "mat",      "maternal",    "contain"};

  assert(!tr.empty());
  assert(tr.size() == 13);
  assert(tr.size("ma") == 7);

  [[maybe_unused]] const auto exact_iter = tr.find("corn");
  assert(exact_iter != tr.end());
  assert(*exact_iter == "corn");

  [[maybe_unused]] const auto prf_iter = tr.find_prefix("mate");
  assert(prf_iter != tr.end());
  assert(*prf_iter == "material");

  [[maybe_unused]] const auto exact_prf_iter = tr.find_prefix("contaminate");
  assert(exact_prf_iter != tr.end());
  assert(*exact_prf_iter == "contaminate");

  [[maybe_unused]] const auto missing_exact_iter = tr.find("testing");
  assert(missing_exact_iter == tr.end());

  [[maybe_unused]] const auto missing_prf_iter = tr.find("conk");
  assert(missing_prf_iter == tr.end());
  cout << " passed\n";
}

void unit_test::insert() {
  cout << "Insert test";
  trie tr;

  [[maybe_unused]] auto iter = tr.insert("math");
  assert(iter != tr.end());
  assert(*iter == "math");
  assert(tr.size("math") == 1);
  assert(!tr.empty("mat"));

  iter = tr.insert("malleable");
  assert(iter != tr.end());
  assert(*iter == "malleable");
  assert(tr.size() == 2);
  assert(!tr.empty("ma"));

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
  trie tr{"mahogany", "mahjong",     "compute", "computer", "matrix",
          "math",     "contaminate", "corn",    "corner",   "material",
          "mat",      "maternal",    "contain"};

  // Erase something that does not exist.
  tr.erase_prefix("random");
  tr.erase("cplusplus");
  assert(tr.size() == 13);

  // Erase a leaf node.
  tr.erase("maternal");
  assert(tr.size() == 12);
  assert(!tr.empty());
  assert(tr.find("maternal") == tr.end());
  assert(tr.size("mat") == 4);
  assert(tr.empty("matern"));

  // Erase non-degenerate internal node.
  tr.erase("mat");
  auto iter = tr.find_prefix("mat");
  assert(iter != tr.end());
  assert(*iter == "material");
  assert(tr.size("ma") == 5);
  assert(!tr.empty("mat"));

  // Erase degenerate internal node.
  tr.erase("corn");
  iter = tr.find("corner");
  assert(iter != tr.end());
  assert(*iter == "corner");
  assert(tr.size("co") == 5);

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

  vector<string> words{"compute", "computer", "contain",  "contaminate",
                       "corn",    "corner",   "mahjong",  "mahogany",
                       "mat",     "material", "maternal", "math",
                       "matrix"};

  // Also tests input iterator constructor.
  const trie tr(words.begin(), words.end());
  const auto total_iterated = extract_range(tr.begin(), tr.end());
  assert(words == total_iterated);

  // Only iterate over subportion.
  vector<string> co_words{"compute",     "computer", "contain",
                          "contaminate", "corn",     "corner"};
  const auto co_iterated = extract_range(tr.begin("co"), tr.end("co"));
  assert(co_words == co_iterated);

  vector<string> ma_words{"mahjong",  "mahogany", "mat",   "material",
                          "maternal", "math",     "matrix"};
  const auto ma_iterated = extract_range(tr.begin("ma"), tr.end("ma"));
  assert(ma_words == ma_iterated);

  // Prefix subportion.
  vector<string> mate_words{"material", "maternal"};
  const auto mate_iterated = extract_range(tr.begin("mate"), tr.end("mate"));
  assert(mate_words == mate_iterated);

  // Singular word range.
  [[maybe_unused]] const auto single_start = tr.begin("contaminate");
  [[maybe_unused]] const auto single_finish = tr.end("contaminate");
  assert(single_start != tr.end());
  assert(*single_start == "contaminate");
  assert(single_finish != tr.end());
  assert(*single_finish == "corn");

  // Non-existant range.
  assert(tr.begin("cops") == tr.end());
  assert(*tr.end("cops") == "corn");
  cout << " passed\n";
}

void unit_test::reverse_iterate() {
  cout << "Reverse iteration test";

  vector<string> words{"matrix", "math",        "maternal", "material",
                       "mat",    "mahogany",    "mahjong",  "corner",
                       "corn",   "contaminate", "contain",  "computer",
                       "compute"};
  // Also tests input iterator constructor.
  const trie tr(words.begin(), words.end());
  // TODO(andrew): this test is invalid and relies on implementation details.
  // It's just here so I can validate this commit.
  size_t i = 0;
  for (auto it = tr.find("matrix"); it != tr.end(); --it) {
    assert(*it == words[i]);
    ++i;
  }
  cout << " passed\n";
}

void unit_test::copy_move() {
  cout << "Copy and Move test";

  trie original{"mahogany", "mahjong",     "compute", "computer", "matrix",
                "math",     "contaminate", "corn",    "corner",   "material",
                "mat",      "maternal",    "contain"};
  const auto orig_vec = extract_range(original.begin(), original.end());

  trie copied(original);
  const auto copy_ctor_vec = extract_range(copied.begin(), copied.end());
  assert(orig_vec == copy_ctor_vec);

  copied.clear();
  copied = original;
  const auto copy_assign_vec = extract_range(copied.begin(), copied.end());
  assert(orig_vec == copy_assign_vec);

  trie moved{std::move(original)};
  const auto move_ctor_vec = extract_range(moved.begin(), moved.end());
  assert(orig_vec == move_ctor_vec);

  copied.clear();
  moved = std::move(copied);
  const auto move_assign_vec = extract_range(moved.begin(), moved.end());
  assert(move_assign_vec.empty());
  cout << " passed\n";
}

void unit_test::comparison() {
  cout << "Comparison test";

  trie t1{"mahogany", "mahjong",     "compute", "computer", "matrix",
          "math",     "contaminate", "corn",    "corner",   "material",
          "mat",      "maternal",    "contain"};

  trie t2{"compute",  "computer", "contain",  "contaminate", "corn",
          "corner",   "mahjong",  "mahogany", "mat",         "material",
          "maternal", "math",     "matrix"};

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

void unit_test::run_all() {
  cout << "--- EXECUTING UNIT TESTS ---\n";
  empty_single();
  find();
  insert();
  erase();
  forward_iterate();
  reverse_iterate();
  copy_move();
  comparison();
  arithmetic();
  cout << "--- FINISHED UNIT TESTS ---\n";
}
