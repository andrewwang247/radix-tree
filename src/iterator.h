/*
Copyright 2026. Andrew Wang.

Interface for Trie iterator.
*/
#pragma once

#include <iterator>
#include <memory>
#include <string>

#include "node.h"

/**
 * @brief Supports const bidirectional iteration over the trie.
 */
class iterator {
 public:
  using iterator_category = std::bidirectional_iterator_tag;
  using value_type = std::string;
  using difference_type = std::ptrdiff_t;
  using pointer = const std::string*;
  using reference = const std::string&;

 private:
  const node* root = nullptr;
  const node* ptr = nullptr;

  /**
   * @brief Proxy to make the arrow operator work.
   */
  struct arrow_proxy {
    value_type value;
    pointer operator->() const { return &value; }
  };

 public:
  iterator() noexcept = default;

  /**
   * @brief Root and pointer constructor.
   * @param rt The root node of the trie.
   * @param p The node that the iterator is currently pointing at.
   */
  iterator(const std::unique_ptr<node>& rt, const node* p) noexcept;

  /**
   * @brief Root and pointer constructor.
   * @param rt The root node of the trie.
   * @param p The node that the iterator is currently pointing at.
   */
  iterator(const std::unique_ptr<node>& rt,
           const std::unique_ptr<node>& p) noexcept;

  /**
   * @brief Prefix increment.
   * @return The next iterator.
   */
  iterator& operator++() noexcept;

  /**
   * @brief Postfix increment.
   * @return The current iterator.
   */
  iterator operator++(int) noexcept;

  /**
   * @brief Prefix decrement.
   * @return The next iterator.
   */
  iterator& operator--() noexcept;

  /**
   * @brief Postfix decrement.
   * @return The current iterator.
   */
  iterator operator--(int) noexcept;

  /**
   * @brief Dereference operator.
   * @return The string referred to by this.
   */
  value_type operator*() const;

  /**
   * @brief Arrow operator. Alias for dereference.
   * @return The string referred to by this.
   */
  arrow_proxy operator->() const;

  /**
   * @brief Implicit conversion to bool.
   * @return Whether or not the underlying pointer is null.
   */
  operator bool() const noexcept;

  /**
   * @brief Convert the trie rooted at this to a JSON object.
   * @param include_ends Include is_end markers in the JSON output.
   * @return A JSON object representing the sub-trie at this.
   */
  std::string to_json(bool include_ends = false) const;

  /**
   * @brief Check if two iterators are equal.
   * @param lhs The left iterator.
   * @param rhs The right iterator.
   * @return Equality between lhs and rhs.
   */
  friend bool operator==(const iterator& lhs, const iterator& rhs) noexcept;
};
