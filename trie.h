/*
Copyright 2026. Andrew Wang.

Interface for Trie.
*/
#pragma once
#include <cassert>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <string>

#include "iterator.h"
#include "node.h"

/**
 * @brief A compact prefix tree with keys as std::string. The empty string
 * is always contained in the trie.
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
  explicit trie(const std::initializer_list<std::string>& key_list);

  /**
   * @brief Range constructor inserts strings contained in [first, last) into
   * trie. Duplicates are ignored.
   * @param first The starting iterator of the range.
   * @param last The ending iterator (one past end) of the range.
   */
  template <typename InputIterator>
  trie(InputIterator first, InputIterator last);

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
  explicit trie(std::unique_ptr<node>&& cloned);

 public:
  /* --- CONTAINER SIZE --- */

  /**
   * @brief Check if the trie is empty.
   * @param prefix The prefix on which to check for emptiness.
   * @return Whether or not the trie is empty starting at given prefix.
   * Prefix defaults to empty string, corresponding to entire trie.
   */
  bool empty(std::string prefix = "") const;

  /**
   * @brief Get the size of the trie under the prefix.
   * @param prefix The prefix on which to check for size.
   * @return The number of words stored in the trie with given prefix.
   * Default prefix is empty, which means the full trie size is returned.
   */
  size_t size(std::string prefix = "") const;

  /* --- ITERATION --- */

  /**
   * @brief Standard begin iterator getter.
   * @return Iterator to the beginning of the trie.
   */
  iterator begin() const;

  /**
   * @brief Standard end iterator getter.
   * @return Iterator to one past the end of the trie.
   */
  iterator end() const;

  /*
  Prefix traversal by iterator. Returns begin and end iterators to the range of
  items which has prefix given by the parameter. Note that they constitute an
  alphabetically ordered range like regular traversal by iterator.
  If none of the keys have the given prefix, returns a null iterator.
  begin("") and end("") have the same behavior as begin() and end()
  since every key has empty string as prefix.
  */

  /**
   * @brief Prefix ranged begin iterator.
   * @param prefix The prefix to obtain a begin iterator for.
   * @return Iterator to the start of the range with given prefix.
   */
  iterator begin(const std::string& prefix) const;

  /**
   * @brief Prefix ranged end iterator.
   * @param prefix The prefix to obtain an end iterator fpr.
   * @return Iterator to one past the end of the range with given prefix.
   */
  iterator end(std::string prefix) const;

  /* --- SEARCHING --- */

  /**
   * @brief Searches for key in trie.
   * @param key The key used to search the trie.
   * @return An iterator to key if it exists. Otherwise, returns a null
   * iterator.
   */
  iterator find(const std::string& key) const;

  /**
   * @brief Searches for prefix in trie.
   * @param prefix The prefix used to search the trie.
   * @return An iterator to the first key that matches the prefix. Otherwise,
   * null iterator.
   */
  iterator find_prefix(std::string prefix) const;

  /* --- INSERTION --- */

  /**
   * @brief Inserts key (or key pointed to by iterator) into trie. Idempotent if
   * key already in trie.
   * @param key The key to insert into the trie.
   * @return An iterator to the key (whether inserted or not).
   */
  iterator insert(std::string key);

  /* --- DELETION --- */

  /**
   * @brief Erases key from trie. Idempotent if key is not in trie.
   * @param key The key to erase from the trie.
   */
  void erase(std::string key);

  /**
   * @brief Erases all keys with prefix from trie. Idempotent if prefix is not
   * in trie.
   * @param prefix The prefix to erase from the trie.
   */
  void erase_prefix(std::string prefix);

  /**
   * @brief Erases all keys from trie. Idempotent on empty tries.
   */
  void clear();

  /* --- REPRESENTATION --- */

  /**
   * @brief Convert the trie to a JSON object.
   * @param include_ends Include is_end markers in the JSON output.
   * @return A JSON object representing the trie's structure.
   */
  std::string to_json(bool include_ends = false) const;

  /* --- ASYMMETRIC BINARY OPERATIONS --- */

  /*
  A + B inserts all of B's keys into A.
  A - B erases all of B's keys from A.
  */

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

  // Private access for == operator to allow efficient deep equality check.
  friend bool operator==(const trie& lhs, const trie& rhs);
};

/* --- SYMMETRIC BINARY OPERATIONS --- */

/*
COMPARISON OF TRIES.
We say that A == B if A and B have equivalent keys.
Define A < B as a proper subset relation.
Note: operator== is a friend to take advantage of
efficient private functions.
*/

bool operator<(const trie& lhs, const trie& rhs);
bool operator!=(const trie& lhs, const trie& rhs);
bool operator>(const trie& lhs, const trie& rhs);
bool operator<=(const trie& lhs, const trie& rhs);
bool operator>=(const trie& lhs, const trie& rhs);

// Arithmetic operators, uses += and -=.

trie operator+(trie lhs, const trie& rhs);
trie operator-(trie lhs, const trie& rhs);

// TEMPLATED IMPLEMENTATIONS

template <typename InputIterator>
trie::trie(InputIterator first, InputIterator last) : trie() {
  for (InputIterator iter = first; iter != last; ++iter) {
    insert(*iter);
  }
  root->assert_invariants();
}
