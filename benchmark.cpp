/*
Copyright 2020. Siwei Wang.

Unit and performance tests for Trie.
*/
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <set>
#include <type_traits>
#include <vector>

#include "trie.h"

using std::cout;
using std::endl;
using std::function;
using std::ifstream;
using std::is_same;
using std::mismatch;
using std::runtime_error;
using std::set;
using std::string;
using std::vector;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::nanoseconds;
using std::chrono::time_point;
using time_unit = std::chrono::microseconds;

bool is_prefix(const string& prf, const string& word);

vector<string> read_words(const string& perf_word_list, size_t num_perf_words);

void print_duration(time_point<high_resolution_clock, nanoseconds> start,
                    time_point<high_resolution_clock, nanoseconds> finish);

namespace Unit_Test {
bool Empty_Test();
bool Find_Test();
bool Insert_Test();
bool Erase_Test();
bool Iteration_Test();
bool Copy_Test();
bool Comparison_Test();
bool Arithmetic_Test();
}  // namespace Unit_Test

namespace Perf_Test {
// Returns a Container with the words in word_list.
template <class Container>
Container get_words(const vector<string>& word_list);

// Prefix counting test.
void Count_Test(const set<string>& words);
void Count_Test(const Trie& words);

// Prefix finding text.
void Find_Test(const set<string>& words, string prefix);
void Find_Test(const Trie& words, string prefix);

// Prefix erasing test.
void Erase_Test(set<string> words, string prefix);
void Erase_Test(Trie words, string prefix);

// Iteration speed test.
template <typename Container>
void Iterate_Test(const Container& words);
}  // namespace Perf_Test

int main() {
  vector<function<bool()>> test_cases{
      Unit_Test::Empty_Test,      Unit_Test::Find_Test,
      Unit_Test::Insert_Test,     Unit_Test::Erase_Test,
      Unit_Test::Iteration_Test,  Unit_Test::Copy_Test,
      Unit_Test::Comparison_Test, Unit_Test::Arithmetic_Test};

  cout << "--- EXECUTING UNIT TESTS ---\n";
  __uint16_t passed = 0;
  for (auto& test : test_cases) {
    if (test()) {
      ++passed;
      cout << " passed." << endl;
    } else {
      cout << " failed." << endl;
    }
  }

  cout << "Passed " << passed << " out of " << test_cases.size()
       << " unit tests.\n";
  cout << "--- FINISHED UNIT TESTS ---\n" << endl;

  cout << "--- EXECUTING PERFORMANCE TEST ---\n";
  //* Make sure that this announcement matches time_units.
  cout << "Time measured in microseconds.\n";
  const auto master_list = read_words("words.txt", 466551);
  cout << '\n';

  // Insert perf
  auto word_set = Perf_Test::get_words<set<string>>(master_list);
  auto word_trie = Perf_Test::get_words<Trie>(master_list);
  cout << '\n';

  // Count perf
  Perf_Test::Count_Test(word_set);
  Perf_Test::Count_Test(word_trie);
  cout << '\n';

  // Find perf
  Perf_Test::Find_Test(word_set, "re");
  Perf_Test::Find_Test(word_trie, "re");
  cout << '\n';

  // Erase perf
  Perf_Test::Erase_Test(word_set, "pr");
  Perf_Test::Erase_Test(word_trie, "pr");
  cout << '\n';

  // Iteration perf
  Perf_Test::Iterate_Test(word_set);
  Perf_Test::Iterate_Test(word_trie);

  cout << "--- FINISHED PERFORMANCE TEST ---\n" << endl;

  if (equal(word_set.begin(), word_set.end(), word_trie.begin(),
            word_trie.end()))
    cout << "--- FINAL COMPARISON CHECK PASSED ---" << endl;
  else
    cout << "--- FINAL COMPARISON CHECK FAILED ---" << endl;
}

bool is_prefix(const string& prf, const string& word) {
  // The empty string is a prefix for every string.
  if (prf.empty()) return true;
  // Assuming non-emptiness of prf, it cannot be longer than word.
  if (prf.length() > word.length()) return false;
  // std::algorithm function that returns iterators to first mismatch.
  auto res = mismatch(prf.cbegin(), prf.cend(), word.cbegin());

  // If we reached the end of prf, it's a prefix.
  return res.first == prf.cend();
}

vector<string> read_words(const string& perf_word_list, size_t num_perf_words) {
  ifstream fin(perf_word_list);
  if (!fin) throw runtime_error("Could not open words.txt");

  cout << "Importing words.txt...\n";
  vector<string> master_list;
  master_list.reserve(num_perf_words);
  for (string word; fin >> word;) {
    master_list.push_back(word);
  }
  fin.close();
  return master_list;
}

void print_duration(time_point<high_resolution_clock, nanoseconds> start,
                    time_point<high_resolution_clock, nanoseconds> finish) {
  cout << "Duration: " << duration_cast<time_unit>(finish - start).count()
       << endl;
}

bool Unit_Test::Empty_Test() {
  cout << "Empty test";
  Trie tr;
  if (!tr.empty()) return false;
  if (!tr.empty("hello")) return false;
  if (tr.size() != 0) return false;
  if (tr.size("world") != 0) return false;
  if (tr.find("test") != tr.end()) return false;
  return true;
}

bool Unit_Test::Find_Test() {
  cout << "Find test";
  Trie tr{"mahogany", "mahjong",     "compute", "computer", "matrix",
          "math",     "contaminate", "corn",    "corner",   "material",
          "mat",      "maternal",    "contain"};

  if (tr.empty()) return false;
  if (tr.size() != 13) return false;
  if (tr.size("ma") != 7) return false;

  auto exact_iter = tr.find("corn");
  if (exact_iter == tr.end() || *exact_iter != "corn") return false;

  auto prf_iter = tr.find("mate", Trie::PREFIX_FLAG);
  if (prf_iter == tr.end() || *prf_iter != "material") return false;

  auto exact_prf_iter = tr.find("contaminate", Trie::PREFIX_FLAG);
  if (exact_prf_iter == tr.end() || *exact_prf_iter != "contaminate")
    return false;

  auto missing_exact_iter = tr.find("testing");
  if (missing_exact_iter != tr.end()) return false;

  auto missing_prf_iter = tr.find("conk");
  if (missing_prf_iter != tr.end()) return false;

  return true;
}

bool Unit_Test::Insert_Test() {
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

bool Unit_Test::Erase_Test() {
  cout << "Erase test";
  Trie tr{"mahogany", "mahjong",     "compute", "computer", "matrix",
          "math",     "contaminate", "corn",    "corner",   "material",
          "mat",      "maternal",    "contain"};

  // Erase something that does not exist.
  tr.erase("random", Trie::PREFIX_FLAG);
  tr.erase("cplusplus");
  if (tr.size() != 13) return false;

  // Erase a leaf node.
  tr.erase("maternal");
  if (tr.size() != 12 || tr.empty()) return false;
  if (tr.find("maternal") != tr.end()) return false;
  if (tr.size("mat") != 4 || !tr.empty("matern")) return false;

  // Erase non-degenerate internal node.
  tr.erase("mat");
  auto iter = tr.find("mat", Trie::PREFIX_FLAG);
  if (iter == tr.end() || *iter != "material") return false;
  if (tr.size("ma") != 5 || tr.empty("mat")) return false;

  // Erase degenerate internal node.
  tr.erase("corn");
  iter = tr.find("corner");
  if (iter == tr.end() || *iter != "corner") return false;
  if (tr.size("co") != 5) return false;

  tr.erase("con", Trie::PREFIX_FLAG);
  if (tr.find("contain") != tr.end()) return false;
  if (tr.find("contaminate") != tr.end()) return false;
  if (tr.find("con", Trie::PREFIX_FLAG) != tr.end()) return false;
  if (tr.size("co") != 3) return false;

  // Try clearing.
  tr.clear();
  if (!tr.empty()) return false;
  if (tr.size() != 0) return false;

  return true;
}

bool Unit_Test::Iteration_Test() {
  cout << "Iteration test";

  vector<string> words{"compute", "computer", "contain",  "contaminate",
                       "corn",    "corner",   "mahjong",  "mahogany",
                       "mat",     "material", "maternal", "math",
                       "matrix"};

  // Also tests input iterator constructor.
  const Trie tr(words.begin(), words.end());

  vector<string> total_iterated(tr.begin(), tr.end());

  if (words != total_iterated) return false;

  // Only iterate over subportion.
  vector<string> co_words{"compute",     "computer", "contain",
                          "contaminate", "corn",     "corner"};
  vector<string> co_iterated(tr.begin("co"), tr.end("co"));
  if (co_words != co_iterated) return false;

  vector<string> ma_words{"mahjong",  "mahogany", "mat",   "material",
                          "maternal", "math",     "matrix"};
  vector<string> ma_iterated(tr.begin("ma"), tr.end("ma"));
  if (ma_words != ma_iterated) return false;

  // Prefix subportion.
  vector<string> mate_words{"material", "maternal"};
  vector<string> mate_iterated(tr.begin("mate"), tr.end("mate"));
  if (mate_words != mate_iterated) return false;

  // Singular word range.
  auto single_start = tr.begin("contaminate");
  auto single_finish = tr.end("contaminate");
  if (single_start == tr.end() || *single_start != "contaminate") return false;
  if (single_finish == tr.end() || *single_finish != "corn") return false;

  // Non-existant range.
  if (tr.begin("cops") != tr.end()) return false;
  if (*tr.end("cops") != "corn") return false;

  return true;
}

bool Unit_Test::Copy_Test() {
  cout << "Copy test";

  Trie original{"mahogany", "mahjong",     "compute", "computer", "matrix",
                "math",     "contaminate", "corn",    "corner",   "material",
                "mat",      "maternal",    "contain"};

  Trie copied(original);

  vector<string> orig_vec(original.begin(), original.end());
  vector<string> copy_vec(copied.begin(), copied.end());

  return orig_vec == copy_vec;
}

bool Unit_Test::Comparison_Test() {
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

bool Unit_Test::Arithmetic_Test() {
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

template <class Container>
Container Perf_Test::get_words(const vector<string>& word_list) {
  // Make announcement.
  if (is_same<Container, set<string>>::value) {
    cout << "Set insertion...\n";
  } else if (is_same<Container, Trie>::value) {
    cout << "Trie insertion...\n";
  } else {
    throw runtime_error("Container must be either set<string> or Trie.");
  }

  // Time insertion with range constructor.
  auto start = high_resolution_clock::now();
  Container words(word_list.begin(), word_list.end());
  auto finish = high_resolution_clock::now();
  print_duration(start, finish);
  return words;
}

void Perf_Test::Count_Test(const set<string>& words) {
  cout << "Set count...\n";

  auto start = high_resolution_clock::now();
  // Get starting iterators on each character.
  vector<set<string>::iterator> bounds;
  for (char c = 'a'; c <= 'z'; ++c) {
    bounds.push_back(lower_bound(words.begin(), words.end(), string(1, c)));
  }
  bounds.push_back(words.end());
  cout << "\tPrinting subset sizes: ";
  for (size_t i = 0; i < bounds.size() - 1; ++i) {
    cout << distance(bounds[i], bounds[i + 1]) << ' ';
  }
  auto finish = high_resolution_clock::now();

  cout << endl;
  print_duration(start, finish);
}

void Perf_Test::Count_Test(const Trie& words) {
  cout << "Trie count...\n";

  auto start = high_resolution_clock::now();
  cout << "\tPrinting subset sizes: ";
  for (char c = 'a'; c <= 'z'; ++c) {
    cout << words.size(string(1, c)) << ' ';
  }
  auto finish = high_resolution_clock::now();

  cout << endl;
  print_duration(start, finish);
}

void Perf_Test::Find_Test(const set<string>& words, string prefix) {
  cout << "Set find...\n";

  auto t0 = high_resolution_clock::now();

  auto start = lower_bound(words.begin(), words.end(), prefix);
  auto finish = start;
  while (finish != words.end() && is_prefix(prefix, *finish)) {
    ++finish;
  }

  auto t1 = high_resolution_clock::now();
  cout << "Prefix " << prefix << " starts at " << *start << " and ends at "
       << *finish << endl;
  print_duration(t0, t1);
}

void Perf_Test::Find_Test(const Trie& words, string prefix) {
  cout << "Trie find...\n";

  auto t0 = high_resolution_clock::now();

  auto start = words.begin(prefix);
  auto finish = words.end(prefix);

  auto t1 = high_resolution_clock::now();
  cout << "Prefix " << prefix << " starts at " << *start << " and ends at "
       << *finish << endl;
  print_duration(t0, t1);
}

void Perf_Test::Erase_Test(set<string> words, string prefix) {
  cout << "Set deletion...\n";

  auto t0 = high_resolution_clock::now();

  // Find the first item that's a prefix using lower bound.
  auto start = lower_bound(words.begin(), words.end(), prefix);
  // Find where it stops being a prefix.
  auto finish = start;
  while (finish != words.end() && is_prefix(prefix, *finish)) {
    ++finish;
  }
  words.erase(start, finish);

  auto t1 = high_resolution_clock::now();

  cout << "Erased all words with prefix " << prefix << endl;
  print_duration(t0, t1);
}

void Perf_Test::Erase_Test(Trie words, string prefix) {
  cout << "Trie deletion...\n";

  auto t0 = high_resolution_clock::now();
  words.erase(prefix, Trie::PREFIX_FLAG);
  auto t1 = high_resolution_clock::now();

  cout << "Erased all words with prefix " << prefix << endl;
  print_duration(t0, t1);
}

template <class Container>
void Perf_Test::Iterate_Test(const Container& words) {
  if (is_same<Container, set<string>>::value) {
    cout << "Set iteration...\n";
  } else if (is_same<Container, Trie>::value) {
    cout << "Trie iteration...\n";
  } else {
    throw runtime_error("Container must be either set<string> or Trie.");
  }

  size_t counter = 0;
  auto t0 = high_resolution_clock::now();
  for (const auto& key : words) {
    /*
    Work-around for compiler flags not using the key variable.
    Branch prediction should optimize out this call.
    */
    if (!key.empty()) ++counter;
  }
  auto t1 = high_resolution_clock::now();
  cout << "Finished iterating over " << counter << " keys.\n";
  print_duration(t0, t1);
}
