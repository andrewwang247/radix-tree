/*
Compressed radix tree.

Copyright 2026. Andrew Wang.
*/
#pragma once
#include <compare>
#include <concepts>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <ranges>
#include <string>
#include <string_view>

#include "iterator.h"
#include "node.h"

namespace rt {

template <typename T>
concept sv_like = std::convertible_to<T, std::string_view>;

// NOLINTBEGIN
template <typename T>
concept sv_range =
    std::ranges::input_range<T> && sv_like<std::ranges::range_reference_t<T>>;
// NOLINTEND

/**
 * @brief A compact prefix tree with keys as std::string.
 *
 * The empty string is always contained in the trie.
 */
class trie {
 private:
  std::unique_ptr<node> root;

 public:
  /**
   * @brief Default constructor initializes empty trie.
   */
  trie();

  /**
   * @brief Copy constructor.
   * @param other The trie to copy into this.
   */
  trie(const trie& other);

  /**
   * @brief Assignment for both copy and move.
   * @param other The trie to assign to this.
   */
  trie& operator=(trie other);

  /**
   * @brief Move constructor.
   * @param other The trie to move into this.
   */
  trie(trie&& other) = default;

  /**
   * @brief Initializer list constructor inserts strings in key_list into trie.
   * Duplicates are ignored.
   * @param key_list The items to initialize the trie with.
   */
  explicit trie(const std::initializer_list<std::string_view>& key_list);

  /**
   * @brief Range constructor inserts strings contained in range into
   * trie. Duplicates are ignored.
   * @param rng The input string range to insert.
   */
  explicit trie(sv_range auto&& rng);

  // --- CONTAINER SIZE ---

  /**
   * @brief Check if the trie is empty.
   * @param prefix The prefix on which to check for emptiness.
   * @return Whether or not the trie is empty starting at given prefix.
   * Prefix defaults to empty string, corresponding to entire trie.
   */
  bool empty(std::string_view prefix = "") const noexcept;

  /**
   * @brief Get the size of the trie under the prefix.
   * @param prefix The prefix on which to check for size.
   * @return The number of words stored in the trie with given prefix.
   * Default prefix is empty, which means the full trie size is returned.
   */
  std::size_t size(std::string_view prefix = "") const noexcept;

  // --- ITERATION ---

  /**
   * @brief Standard begin iterator getter.
   * @return Iterator to the beginning of the trie.
   */
  iterator begin() const noexcept;

  /**
   * @brief Standard end iterator getter.
   * @return Iterator to one past the end of the trie.
   */
  iterator end() const noexcept;

  // --- KEY SEARCHING ---

  /**
   * @brief Checks for key in trie.
   * @param key The key to check in trie.
   * @return Whether key is contained in trie.
   */
  bool contains(std::string_view key) const noexcept;

  /**
   * @brief Searches for key in trie.
   * @param key The key used to search the trie.
   * @return Iterator to key if it exists. Otherwise, null iterator.
   */
  iterator find(std::string_view key) const noexcept;

  // --- PREFIX SEARCHING ---

  /**
   * @brief Checks for any key containing prefix in trie.
   * @param prefix The prefix to check in trie.
   * @return Whether any key with prefix is contained in trie.
   */
  bool contains_prefix(std::string_view prefix) const noexcept;

  /**
   * @brief Get range of keys with a given prefix.
   * @param prefix The prefix to range over.
   * @return Range over all keys with prefix.
   */
  std::ranges::bidirectional_range auto find_prefix(
      std::string_view prefix) const noexcept;

 private:
  // Note that the find_prefix begin and end iterators are not necessarily well
  // behaved as a range. For example, if a word shares a common prefix with some
  // keys but is not itself a prefix. Begin does not find any range for which
  // word is a prefix and returns a sentinel. End points to the start of the
  // prefix range after word. This creates a begin > end scenario that we do not
  // wish to expose as part of the public API.

  /**
   * @brief Prefix ranged begin iterator.
   * @param prefix The prefix to obtain a begin iterator for.
   * @warning Do NOT iterate over find_prefix begin and end. Use subrange
   * function.
   * @return Iterator to the start of the range with given prefix.
   */
  iterator find_prefix_begin(std::string_view prefix) const noexcept;

  /**
   * @brief Prefix ranged end iterator.
   * @param prefix The prefix to obtain an end iterator for.
   * @warning Do NOT iterate over find_prefix begin and end. Use subrange
   * function.
   * @return Iterator to one past the end of the range with given prefix.
   */
  iterator find_prefix_end(std::string_view prefix) const noexcept;

 public:
  // --- INSERTION ---

  /**
   * @brief Inserts key (or key pointed to by iterator) into trie. Idempotent if
   * key already in trie.
   * @param key The key to insert into the trie.
   * @return An iterator to the key (whether inserted or not).
   */
  iterator insert(std::string_view key);

  /**
   * @brief Inserts a range of keys into trie.
   * @param rng The range of keys to insert.
   */
  void insert_range(sv_range auto&& rng);

  // --- DELETION ---

  /**
   * @brief Erases key from trie. Idempotent if key is not in trie.
   * @param key The key to erase from the trie.
   */
  void erase(std::string_view key);

  /**
   * @brief Erases all keys with prefix from trie. Idempotent if prefix is not
   * in trie.
   * @param prefix The prefix to erase from the trie.
   */
  void erase_prefix(std::string_view prefix);

  /**
   * @brief Erase a range of keys from trie.
   * @param rng The range of keys to erase.
   */
  void erase_range(sv_range auto&& rng);

  /**
   * @brief Erases all keys from trie. Idempotent on empty tries.
   */
  void clear() noexcept;

  // --- REPRESENTATION ---

  /**
   * @brief Convert the trie to a JSON object.
   * @param include_ends Include is_end markers in the JSON output.
   * @return A JSON object representing the trie's structure.
   */
  std::string to_json(bool include_ends = false) const;

  // --- VALIDATION AND DEBUGGING ---

  /**
   * @brief Assert that trie satisfies invariants.
   * @pre DEBUG is defined.
   */
  void assert_invariants() const noexcept;

  // --- ASYMMETRIC BINARY OPERATIONS ---

  /**
   * @brief Inserts all of other's keys into this.
   * @param other The trie to union with this.
   * @return A reference to this.
   */
  trie& operator+=(const trie& other);

  /**
   * @brief Removes all of other's keys from this.
   * @param other The trie to set subtract from this.
   * @return A reference to this.
   */
  trie& operator-=(const trie& other);

  /**
   * @brief Equality operator checks element-wise equality. Private access
   * permits efficient traversal.
   * @param lhs The left trie.
   * @param rhs The right trie.
   * @return Whether the 2 tries have matching content.
   */
  friend bool operator==(const trie& lhs, const trie& rhs) noexcept;
};

// --- SYMMETRIC BINARY OPERATIONS ---

/**
 * @brief Set union of two tries.
 * @param lhs The left trie to union.
 * @param rhs The right trie to union.
 * @return A trie with the union of keys.
 */
trie operator+(trie lhs, const trie& rhs);

/**
 * @brief Set difference of two tries.
 * @param lhs The minuend to subtract from.
 * @param rhs The subtrahend to take away.
 * @return A trie lhs keys that aren't in rhs.
 */
trie operator-(trie lhs, const trie& rhs);

/**
 * @brief Three way comparison operator for subset partial ordering.
 * @param lhs The left trie.
 * @param rhs The right trie.
 * @return Subset partial ordering.
 */
std::partial_ordering operator<=>(const trie& lhs, const trie& rhs) noexcept;

// TEMPLATED AND AUTO RETURN IMPLEMENTATIONS

trie::trie(sv_range auto&& rng) : trie{} { insert_range(rng); }

void trie::insert_range(sv_range auto&& rng) {
  for (auto&& key : rng) {
    insert(key);
  }
}

void trie::erase_range(sv_range auto&& rng) {
  for (auto&& key : rng) {
    erase(key);
  }
}

inline std::ranges::bidirectional_range auto trie::find_prefix(
    std::string_view prefix) const noexcept {
  const auto begin_rng = find_prefix_begin(prefix);
  const auto sentinel = end();
  return begin_rng == sentinel
             ? std::ranges::subrange{sentinel, sentinel}
             : std::ranges::subrange{begin_rng, find_prefix_end(prefix)};
}

}  // namespace rt

// Disable sized range concept in global namespace

template <>
inline constexpr bool std::ranges::disable_sized_range<rt::trie> = true;
