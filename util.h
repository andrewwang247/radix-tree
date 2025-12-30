/*
Copyright 2026. Andrew Wang.

Utility function definitions.
*/
#pragma once
#include <random>
#include <string>
#include <vector>

// Name of the word list file.
static constexpr auto WORD_LIST_FILE = "words.txt";
// Number of words in the file.
static constexpr size_t WORD_LIST_SIZE = 466478;
// Singleton random device for seed generation.
static std::random_device RANDOM_DEVICE{};

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
