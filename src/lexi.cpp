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
  auto local = string{word};
  if (const auto lnm = local.find_last_not_of(numeric_limits<char>::max());
      lnm != string::npos) {
    // Increment last non max char and remove everything after.
    ++local[lnm];
    local.erase(lnm + 1);
  } else {
    // All characters are max char. Append min char.
    local += numeric_limits<char>::min();
  }
  return local;
}

}  // namespace rt
