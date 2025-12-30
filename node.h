/*
Copyright 2026. Andrew Wang.

Interface for Node.
*/
#pragma once
#include <map>
#include <memory>
#include <string>

/**
 * @brief Defines a singular node in the Trie data structure.
 */
class Node {
 public:
  bool is_end;
  const Node* parent;
  std::map<std::string, std::unique_ptr<Node>> children;
  /**
   * @brief Construct a new node with no children.
   * @param is_end The is_end value.
   * @param parent The parent pointer.
   */
  Node(bool is_end, const Node* parent);

  /**
   * @brief Recursively copies other into this.
   */
  void copy_from(const Node* other);

  /**
   * @brief Deep equality check.
   * @param other: The non-null root of the other trie.
   * @return Whether or not the tries rooted at this and other are equivalent.
   */
  bool equals(const Node* other) const;

  /**
   * @brief Depth traversing search for the deepest node N such that a prefix of
   * key matches the string representation at N.
   * @param key The key on which to make an approximate match. Modifies key
   * such that the string representation at N is removed.
   * @return The node N described above. Since the root node in a Trie is
   * equivalent to the empty string, N is never null.
   */
  const Node* approximate_match(std::string& key) const;

  /**
   * @brief Counts the number of keys stored at or as children.
   * Equivalent to counting the number of true is_end's accessible from this.
   * @return The number of keys stored at or under this.
   */
  size_t key_count() const;

  /**
   * @brief Depth traversing search for the node that serves as a root for prf.
   * @param prf The prefix which the return node should be a root of. Modifies
   * so that the string at prefix_match is removed from prf. Note that if prf is
   * not a prefix, the modified prf reflects as far as it got.
   * @return The deepest node N such that N and all of N's children have prf as
   * prefix. If prf is not a prefix, returns a nullptr.
   */
  const Node* prefix_match(std::string& prf) const;

  /**
   * @brief Depth traversing search for the node that matches word.
   * @param word The string we are trying to match.
   * @return The first node that exactly matches the given word. If no match is
   * found, returns a nullptr.
   */
  const Node* exact_match(std::string word) const;
  /**
   * @brief Find the first child key.
   * @return The first key that's a child of this or nullptr if empty.
   */
  const Node* first_key() const;
  /**
   * @brief Get the next need for in-order traversal.
   * @return The first key AFTER this that is not a child of this. If there
   * isn't such a key, returns nullptr.
   */
  const Node* next_node() const;

  /**
   * @brief Reconstruct string from this.
   * @return The string representation at this.
   */
  std::string underlying_string() const;

  /**
   * @brief Searches for the the given node in children.
   * @param other The Node we are searching for in the map.
   * @return An iterator to the position which matches other. This is the end
   * iterator if other is not in the map.
   */
  std::map<std::string, std::unique_ptr<Node>>::const_iterator find_child(
      const Node* other) const;

  /**
   * @brief This function is only used for testing!
   */
  bool check_invariant() const;
};
