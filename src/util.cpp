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
using std::random_device;
using std::reverse;
using std::runtime_error;
using std::shuffle;
using std::string;
using std::vector;

bool util::is_prefix(const string& prf, const string& word) {
  // The empty string is a prefix for every string.
  if (prf.empty()) return true;
  // Assuming non-emptiness of prf, it cannot be longer than word.
  if (prf.length() > word.length()) return false;
  // std::algorithm function that returns iterators to first mismatch.
  const auto res = mismatch(prf.begin(), prf.end(), word.begin());

  // If we reached the end of prf, it's a prefix.
  return res.first == prf.end();
}

// Singleton random device for seed generation.
static random_device RANDOM_DEVICE{};
static default_random_engine RNG{RANDOM_DEVICE()};

vector<string> util::permuted(vector<string> words) {
  shuffle(words.begin(), words.end(), RNG);
  return words;
}

vector<string> util::reversed(vector<string> words) {
  reverse(words.begin(), words.end());
  return words;
}

// Name of the word list file.
static constexpr auto WORD_LIST_FILE = "words.txt";
// Number of words in the file.
static constexpr size_t WORD_LIST_SIZE = 466478;

vector<string> util::read_words() {
  vector<string> master_list;
  master_list.reserve(WORD_LIST_SIZE);

  ifstream fin{WORD_LIST_FILE};
  if (!fin) throw runtime_error("Could not open words.txt");
  for (string word; fin >> word;) {
    master_list.push_back(word);
  }
  shuffle(master_list.begin(), master_list.end(), RNG);

  cout << "Imported " << master_list.size() << " randomly shuffled words\n";
  return master_list;
}
