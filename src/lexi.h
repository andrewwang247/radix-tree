/**
 * Lexicographic helpers functions.
 *
 * Copyright 2026. Andrew Wang.
 */
#pragma once
#include <string>
#include <string_view>

namespace rt::lexicographic {

/**
 * @brief Increment a string to the next possible in lexicographic order.
 * @param word The current string to process.
 * @return The lexicographical earliest string greater than word.
 */
std::string increment(std::string_view word);

}  // namespace rt::lexicographic
