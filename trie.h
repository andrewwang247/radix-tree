/*
Copyright 2026. Andrew Wang.

Interface for Trie.
*/
#pragma once
#include <cassert>
#include <exception>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <string>

/**
 * @brief A compact prefix tree with keys as std::basic_string. The empty string
 * is always contained in the trie.
 *
 * Radix tree invariants.
 * 1. Given a node N, children of N do not share any common non-empty prefixes.
 *     Otherwise, the common prefix would have been compressed.
 * 2. As a corollary of (1), for any non-empty prefix P and node N, at most 1
 *     child node of N has P as a prefix.
 * 3. The empty string is never in a children map. Suppose N contains the empty
 *     string in its children map. This would be equivalent to N being is_end.
 * 4. All leaf nodes have true is_end. If a leaf node N was not the end of a
 * key, must have non-empty children map, which it can't have because it's a
 * leaf.
 * 5. If node N has false is_end, it must have at least 2 children node.
 *     Otherwise, it would be compressed with its only child.
 * 6. As another corollary of (1), a children map can have at most |char| items.
 *     Therefore, we can treat searching std::map as constant.
 * 7. approximate_match, prefix_match, and exact_match can be composed due
 *     to the recursive structure of the trie.
 * 8. root is never null. The empty trie consists of a root node with false
 * is_end, an empty children map, and nullptr as parent.
 */
class Trie {
 private:
  /**
   * @brief Defines a singular node in the Trie data structure.
   */
  struct Node {
    bool is_end;
    std::weak_ptr<Node> parent;
    std::map<std::string, std::shared_ptr<Node>> children;
    /**
     * @brief Construct a new node with no children.
     * @param is_end_in The is_end value.
     * @param parent_in The parent pointer.
     */
    Node(bool is_end_in, std::shared_ptr<Node> const parent_in);
  };

  std::shared_ptr<Node> root;

  /* --- HELPER FUNCTIONS --- */

  /**
   * @brief Recursively copies other into rt.
   */
  static void recursive_copy(std::shared_ptr<Node> const rt,
                             const std::shared_ptr<Node> other);

  /**
   * @brief Check for prefixes of words.
   * @param prf The string to match with the beginning of word.
   * @param word The full prefix to test.
   * @return whether or not prf is a prefix of word.
   */
  static bool is_prefix(const std::string& prf, const std::string& word);

  /**
   * @brief Depth traversing search for the deepest node N such that a prefix of
   * key matches the string representation at N.
   * @param rt The non-null node at which to start searching.
   * @param key The key on which to make an approximate match. Modifies key
   * such that the string representation at N is removed.
   * @return The node N described above. Since the root node is equivalent to
   * the empty string, N is never null.
   */
  static std::shared_ptr<Node> approximate_match(const std::shared_ptr<Node> rt,
                                                 std::string& key);

  /**
   * @brief Depth traversing search for the node that serves as a root for prf.
   * @param rt The non-null node at which to start searching.
   * @param prf The prefix which the return node should be a root of. Modifies
   * so that the string at prefix_match is removed from prf. Note that if prf is
   * not a prefix, the modified prf reflects as far as it got.
   * @return The deepest node N such that N and all of N's children have prf as
   * prefix. If prf is not a prefix, returns a nullptr.
   */
  static std::shared_ptr<Node> prefix_match(const std::shared_ptr<Node> rt,
                                            std::string& prf);

  /**
   * @brief Depth traversing search for the node that matches word.
   * @param rt The non-null root node from which to search.
   * @param word The string we are trying to match.
   * @return The first node that exactly matches the given word. If no match is
   * found, returns a nullptr.
   */
  static std::shared_ptr<Node> exact_match(const std::shared_ptr<Node> rt,
                                           std::string word);

  /**
   * @brief Counts the number of keys stored at or as children of rt added to
   * acc. Equivalent to counting the number of true is_end's accessible from rt.
   * @param rt The non-null root node at which to start counting.
   * @param acc The value at which to start counting.
   */
  static void key_counter(const std::shared_ptr<Node> rt, size_t& acc);

  /**
   * @brief Deep equality check.
   * @param rt_1: The non-null root of the first trie.
   * @param rt_2: The non-null root of the second trie.
   * @return Whether or not the tries rooted at rt_1 and rt_2 are equivalent.
   */
  static bool are_equal(const std::shared_ptr<Node> rt_1,
                        const std::shared_ptr<Node> rt_2);

  /**
   * @brief Searches for the the given value in a map.
   * @param m The map on which to search.
   * @param val The value we are searching for in the map.
   * @return An iterator to the position which matches val. This is the end
   * iterator if val is not in the map.
   */
  template <typename K, typename V>
  static typename std::map<K, std::shared_ptr<V>>::const_iterator value_find(
      const std::map<K, std::shared_ptr<V>>& m, const std::shared_ptr<V> val);

  /**
   * @brief Find the first child key.
   * @param rt The non-null root node at which to start.
   * @return The first key that's a child of rt or nullptr if empty.
   */
  static std::shared_ptr<Node> first_key(std::shared_ptr<Node> rt);

  /**
   * @brief Get the next need for in-order traversal.
   * @param ptr The non-null starting node position.
   * @return The first key AFTER ptr that is not a child of ptr. If there isn't
   * such a key, returns nullptr.
   */
  static std::shared_ptr<Node> next_node(const std::shared_ptr<Node> ptr);

  /**
   * @brief Reconstruct string from node.
   * @param ptr The node for which we are trying to construct a string.
   * @return The string representation at ptr.
   */
  static std::string underlying_string(std::shared_ptr<Node> ptr);

  /**
   * @brief This function is only used for testing!
   * @param root The root of the tree to check.
   * @return Whether or not the tree at root is valid (satisfies invariants).
   */
  static bool check_invariant(const std::shared_ptr<Node> root);

 public:
  /**
   * Used to mark a parameter as passing in a prefix and not a full key.
   */
  static constexpr bool PREFIX_FLAG = true;

  /**
   * @brief Default constructor initializes empty trie.
   */
  Trie();

  /**
   * @brief Initializer list constructor inserts strings in key_list into trie.
   * Duplicates are ignored.
   * @param key_list The items to initialize the trie with.
   */
  explicit Trie(const std::initializer_list<std::string>& key_list);

  /**
   * @brief Range constructor inserts strings contained in [first, last) into
   * trie. Duplicates are ignored.
   * @param first The starting iterator of the range.
   * @param last The ending iterator (one past end) of the range.
   */
  template <typename InputIterator>
  Trie(InputIterator first, InputIterator last);

  /* --- DYNAMIC MEMORY: RULE OF 5 */

  /**
   * @brief Copy constructor.
   * @param other The trie to copy into this.
   */
  Trie(const Trie& other);

  /**
   * @brief Move constructor.
   * @param other The trie to move into this.
   */
  Trie(Trie&& other);

  /**
   * @brief Assignment operator.
   * @param other The trie to assign to this.
   */
  Trie& operator=(Trie other);

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
   * @brief Supports const forward iteration over the trie.
   */
  class iterator {
    using iterator_category = std::forward_iterator_tag;
    using value_type = std::string;
    using difference_type = std::ptrdiff_t;
    using pointer = const std::string*;
    using reference = const std::string&;
    friend class Trie;

   private:
    std::shared_ptr<Node> ptr;

    /**
     * @brief Constructor, Node ptr is null by default.
     * @param t The trie reference to assign to tree.
     * @param p The Node that the iterator is currently pointing at.
     */
    explicit iterator(const std::shared_ptr<Node> p = nullptr);

   public:
    /**
     * @brief Prefix increment.
     * @return The next iterator.
     */
    iterator& operator++();

    /**
     * @brief Postfix increment.
     * @return The current iterator.
     */
    iterator operator++(int);

    /**
     * @brief Dereference operator.
     * @return The string referred to by this.
     */
    std::string operator*();

    /**
     * @brief Implicit conversion to bool.
     * @return Whether or not the underlying pointer is null.
     */
    operator bool() const;

    /* Comparison between iterators performs element-wise comparison. */

    /**
     * @brief Check if two iterators are equal.
     * @param lhs The left iterator.
     * @param rhs The right iterator.
     * @return Equality between lhs and rhs.
     */
    friend bool operator==(const Trie::iterator& lhs,
                           const Trie::iterator& rhs);

    /**
     * @brief Check if two iterators are unequal.
     * @param lhs The left iterator.
     * @param rhs The right iterator.
     * @return Inequality between lhs and rhs.
     */
    friend bool operator!=(const Trie::iterator& lhs,
                           const Trie::iterator& rhs);
  };

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
  iterator begin(std::string prefix) const;

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
   * @param is_prefix Flags whether or not to treat the key as a prefix.
   * @return An iterator to it if it exists. Otherwise, returns a null iterator.
   * If is_prefix is true, returns an iterator to the first key that matches the
   * prefix.
   */
  iterator find(std::string key, bool is_prefix = !PREFIX_FLAG) const;

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
   * @brief Erases key from trie. If prefix flag is set, erases all keys that
   * have the key as prefix from the trie. Idempotent if key (or prefix) is not
   * in trie.
   * @param key The key to erase from the trie.
   * @param is_prefix Flag for treating key as a prefix.
   */
  void erase(std::string key, bool is_prefix = !PREFIX_FLAG);

  /**
   * @brief Erases all keys from trie. Idempotent on empty tries.
   */
  void clear();

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
  Trie& operator+=(const Trie& rhs);

  /**
   * @brief Removes all of rhs's keys from this. Requires that this and rhs are
   * not the same trie.
   * @param rhs The trie to set subtract from this.
   */
  Trie& operator-=(const Trie& rhs);

  // Private access for == operator to allow efficient deep equality check. See
  // COMPARISON OF TRIES.
  friend bool operator==(const Trie& lhs, const Trie& rhs);
};

/* --- SYMMETRIC BINARY OPERATIONS --- */

/*
COMPARISON OF TRIES.
We say that A == B if A and B have equivalent keys.
Define A < B as a proper subset relation.
Note: operator== is a friend to take advantage of
the more efficient Trie::are_equal function.
*/

bool operator<(const Trie& lhs, const Trie& rhs);
bool operator!=(const Trie& lhs, const Trie& rhs);
bool operator>(const Trie& lhs, const Trie& rhs);
bool operator<=(const Trie& lhs, const Trie& rhs);
bool operator>=(const Trie& lhs, const Trie& rhs);

// Arithmetic operators, uses += and -=.

Trie operator+(Trie lhs, const Trie& rhs);
Trie operator-(Trie lhs, const Trie& rhs);

/**
 * @brief Outputs each entry in tree to os. Each entry is given its own line.
 *
 * @param os The output stream.
 * @param tree The tree to write.
 * @return std::ostream& os
 */
std::ostream& operator<<(std::ostream& os, const Trie& tree);

// TEMPLATED IMPLEMENTATIONS

template <typename InputIterator>
Trie::Trie(InputIterator first, InputIterator last) : Trie() {
  for (InputIterator iter = first; iter != last; ++iter) {
    insert(*iter);
  }
  assert(check_invariant(root));
}

template <typename K, typename V>
typename std::map<K, std::shared_ptr<V>>::const_iterator Trie::value_find(
    const std::map<K, std::shared_ptr<V>>& m, const std::shared_ptr<V> val) {
  for (auto it = m.begin(); it != m.end(); ++it) {
    if (it->second == val) return it;
  }
  return m.end();
}
