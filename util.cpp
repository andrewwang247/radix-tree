/*
Copyright 2026. Andrew Wang.

Utility function implementations.
*/
#include "util.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

using std::cout;
using std::default_random_engine;
using std::ifstream;
using std::mismatch;
using std::runtime_error;
using std::string;
using std::vector;

bool is_prefix(const string& prf, const string& word) {
  // The empty string is a prefix for every string.
  if (prf.empty()) return true;
  // Assuming non-emptiness of prf, it cannot be longer than word.
  if (prf.length() > word.length()) return false;
  // std::algorithm function that returns iterators to first mismatch.
  const auto res = mismatch(prf.begin(), prf.end(), word.begin());

  // If we reached the end of prf, it's a prefix.
  return res.first == prf.end();
}

vector<string> read_words() {
  vector<string> master_list;
  master_list.reserve(WORD_LIST_SIZE);

  ifstream fin{WORD_LIST_FILE};
  if (!fin) throw runtime_error("Could not open words.txt");
  for (string word; fin >> word;) {
    master_list.push_back(word);
  }
  auto rng = default_random_engine{RANDOM_DEVICE()};
  shuffle(master_list.begin(), master_list.end(), rng);

  cout << "Imported " << master_list.size() << " randomly shuffled words\n";
  return master_list;
}
