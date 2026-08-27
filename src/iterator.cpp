/*
Copyright 2026. Andrew Wang.

Implementation for Trie iterator.
*/
#include "iterator.h"

#include <cassert>
#include <memory>
#include <string>

#include "node.h"

using std::string;
using std::unique_ptr;

iterator::iterator(const unique_ptr<node>& rt, const node* p) noexcept
    : root(rt.get()), ptr(p) {
  assert(rt);
}

iterator::iterator(const unique_ptr<node>& rt,
                   const unique_ptr<node>& p) noexcept
    : root(rt.get()), ptr(p.get()) {
  assert(rt);
}

iterator& iterator::operator++() noexcept {
  if (!ptr) return *this;
  ptr = ptr->children.empty() ? ptr->next_node() : ptr->first_key();
  return *this;
}

iterator iterator::operator++(int) noexcept {
  auto temp(*this);
  ++(*this);
  return temp;
}

iterator& iterator::operator--() noexcept {
  if (ptr) {
    // middle of trie
    ptr = ptr->prev_node();
  } else if (!root->children.empty()) {
    // end of trie, multiple keys
    ptr = root->last_key();
  } else if (root->is_end) {
    // end of trie, singleton
    ptr = root;
  }
  // empty trie means no change in ptr
  return *this;
}

iterator iterator::operator--(int) noexcept {
  auto temp(*this);
  --(*this);
  return temp;
}

iterator::value_type iterator::operator*() const {
  return ptr->underlying_string();
}

iterator::arrow_proxy iterator::operator->() const {
  return arrow_proxy{ptr->underlying_string()};
}

iterator::operator bool() const noexcept { return ptr != nullptr; }

string iterator::to_json(bool include_ends) const {
  return ptr ? ptr->to_json(include_ends) : "{}";
}

bool operator==(const iterator& lhs, const iterator& rhs) noexcept {
  return lhs.ptr == rhs.ptr;
}
