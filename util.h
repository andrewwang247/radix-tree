/*
Copyright 2026. Andrew Wang.

Utility function definitions.
*/
#pragma once
#include <algorithm>
#include <iterator>
#include <string>
#include <vector>

namespace util {
/**
 * @brief Check for prefixes of words.
 * @param prf The string to match with the beginning of word.
 * @param word The full prefix to test.
 * @return whether or not prf is a prefix of word.
 */
bool is_prefix(const std::string& prf, const std::string& word);

/**
 * @brief Reads words from the WORD_LIST_FILE into a vector of strings. Randomly
 * permutes before returning.
 * @return A vector of strings containing all words from the file.
 */
std::vector<std::string> read_words();

/**
 * Extract the iterator range into a vector.
 * @param begin The beginning iterator.
 * @param end The ending iterator.
 * @return The forward range from begin to end.
 */
template <typename InputIterator>
std::vector<std::string> extract_forward_range(InputIterator begin,
                                               InputIterator end);

/**
 * Extract the iterator range into a vector, moving backwards from end.
 * @param begin The beginning iterator.
 * @param end The ending iterator.
 * @return The reversed range from end to begin.
 */
template <typename BiDirIterator>
std::vector<std::string> extract_reverse_range(BiDirIterator begin,
                                               BiDirIterator end);
}  // namespace util

// TEMPLATED IMPLEMENTATIONS

template <typename InputIterator>
std::vector<std::string> util::extract_forward_range(InputIterator begin,
                                                     InputIterator end) {
  std::vector<std::string> range;
  std::copy(begin, end, std::back_inserter(range));
  return range;
}

template <typename BiDirIterator>
std::vector<std::string> util::extract_reverse_range(BiDirIterator begin,
                                                     BiDirIterator end) {
  std::vector<std::string> range;
  auto iter = end;
  do {
    --iter;
    range.push_back(*iter);
  } while (iter != begin);
  return range;
}
