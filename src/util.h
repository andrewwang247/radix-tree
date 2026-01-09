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
 * @brief Randomly permute the input words.
 * @param items The words to permute.
 * @return Vector containing permuted words.
 */
std::vector<std::string> permuted(std::vector<std::string> words);

/**
 * @brief Reverse the order of the input words.
 * @param items The words to reverse.
 * @return Vector containing reversed words.
 */
std::vector<std::string> reversed(std::vector<std::string> words);

/**
 * Extract the iterator range into a vector, moving backwards from end.
 * @param begin The beginning iterator.
 * @param end The ending iterator.
 * @return The reversed range from end to begin.
 */
template <typename BiDirIterator>
std::vector<std::string> reverse_range(BiDirIterator begin, BiDirIterator end);

/**
 * Verify size and content equality between ranges.
 * @param l_begin Beginning of left range.
 * @param l_end Ending of left range.
 * @param r_begin Beginning of right range.
 * @param r_end Ending of right range.
 * @return If distance and equality are both equal.
 */
template <typename IterLeft, typename IterRight>
bool ranges_equal(IterLeft l_begin, IterLeft l_end, IterRight r_begin,
                  IterRight r_end);

}  // namespace util

// TEMPLATED IMPLEMENTATIONS

template <typename BiDirIterator>
std::vector<std::string> util::reverse_range(BiDirIterator begin,
                                             BiDirIterator end) {
  std::vector<std::string> range;
  auto iter = end;
  do {
    --iter;
    range.push_back(*iter);
  } while (iter != begin);
  return range;
}

template <typename IterLeft, typename IterRight>
bool util::ranges_equal(IterLeft l_begin, IterLeft l_end, IterRight r_begin,
                        IterRight r_end) {
  const auto l_size = std::distance(l_begin, l_end);
  const auto r_size = std::distance(r_begin, r_end);
  return (l_size == r_size) && std::equal(l_begin, l_end, r_begin, r_end);
}
