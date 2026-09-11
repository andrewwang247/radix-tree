/*
Copyright 2026. Andrew Wang.

Interface for Trie.
*/
#pragma once
#include <cassert>
#include <compare>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <ranges>
#include <string>
#include <string_view>

#include "iterator.h"
#include "node.h"

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
   * @brief Initializer list constructor inserts strings in key_list into trie.
   * Duplicates are ignored.
   * @param key_list The items to initialize the trie with.
   */
  explicit trie(const std::initializer_list<std::string_view>& key_list);

  /**
   * @brief Range constructor inserts strings contained in [first, last) into
   * trie. Duplicates are ignored.
   * @param first The starting iterator of the range.
   * @param last The ending iterator (one past end) of the range.
   */
  template <std::input_iterator Iter>
  trie(Iter first, Iter last);

  /**
   * @brief Range constructor inserts strings contained in range into
   * trie. Duplicates are ignored.
   * @param input_range The string range to insert.
   */
  explicit trie(std::ranges::input_range auto&& input_range);

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
   * @brief Default destructor.
   */
  ~trie() = default;

 private:
  /**
   * @brief Private move constructor from a cloned root node.
   * @param cloned The cloned root to move into this.
   */
  explicit trie(std::unique_ptr<node> cloned) noexcept;

 public:
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

  /**
   * @brief Prefix ranged begin iterator.
   * @param prefix The prefix to obtain a begin iterator for.
   * @return Iterator to the start of the range with given prefix.
   */
  iterator begin(std::string_view prefix) const noexcept;

  /**
   * @brief Prefix ranged end iterator.
   * @param prefix The prefix to obtain an end iterator fpr.
   * @return Iterator to one past the end of the range with given prefix.
   */
  iterator end(std::string_view prefix) const noexcept;

  /**
   * @brief Range from begin(prefix) to end(prefix);
   * @param prefix The prefix to range over.
   * @return Subrange over all keys with prefix.
   */
  std::ranges::subrange<iterator> subrange(
      std::string_view prefix) const noexcept;

  // --- SEARCHING ---

  /**
   * @brief Checks for key in trie.
   * @param key The key to check in trie.
   * @return Whether key is contained in trie.
   */
  bool contains(std::string_view key) const noexcept;

  /**
   * @brief Searches for key in trie.
   * @param key The key used to search the trie.
   * @return An iterator to key if it exists. Otherwise, returns a null
   * iterator.
   */
  iterator find(std::string_view key) const noexcept;

  /**
   * @brief Searches for prefix in trie.
   * @param prefix The prefix used to search the trie.
   * @return An iterator to the first key that matches the prefix. Otherwise,
   * null iterator.
   */
  iterator find_prefix(std::string_view prefix) const noexcept;

  // --- INSERTION ---

  /**
   * @brief Inserts key (or key pointed to by iterator) into trie. Idempotent if
   * key already in trie.
   * @param key The key to insert into the trie.
   * @return An iterator to the key (whether inserted or not).
   */
  iterator insert(std::string_view key);

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

  // --- ASYMMETRIC BINARY OPERATIONS ---

  /**
   * @brief Inserts all of rhs's keys into this. Requires that this and rhs are
   * not the same trie.
   * @param rhs The trie to union with this.
   */
  trie& operator+=(const trie& rhs);

  /**
   * @brief Removes all of rhs's keys from this. Requires that this and rhs are
   * not the same trie.
   * @param rhs The trie to set subtract from this.
   */
  trie& operator-=(const trie& rhs);

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

// TEMPLATED IMPLEMENTATIONS

template <std::input_iterator Iter>
trie::trie(Iter first, Iter last) : trie(std::ranges::subrange{first, last}) {}

trie::trie(std::ranges::input_range auto&& input_range) : trie() {
  for (auto&& word : input_range) {
    insert(word);
  }
  root->assert_invariants();
}
