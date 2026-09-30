/**
 * Lexicographic helpers functions.
 *
 * Copyright 2026. Andrew Wang.
 */
#include "lexi.h"

#include <limits>
#include <string>
#include <string_view>

using std::numeric_limits;
using std::string;
using std::string_view;

namespace rt {

string lexicographic::increment(string_view word) {
  auto local_word = string{word};
  const auto last_non_max =
      local_word.find_last_not_of(numeric_limits<char>::max());
  if (last_non_max != string::npos) {
    // Increment last non max char and remove everything after.
    ++local_word[last_non_max];
    local_word.erase(last_non_max + 1);
  } else {
    // All characters are max char. Append min char.
    local_word += numeric_limits<char>::min();
  }
  return local_word;
}

}  // namespace rt
