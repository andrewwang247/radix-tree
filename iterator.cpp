/*
Copyright 2026. Andrew Wang.

Implementation for Trie iterator.
*/
#include "iterator.h"

#include <cassert>
#include <memory>
#include <stdexcept>
#include <string>

using std::runtime_error;
using std::string;
using std::unique_ptr;

iterator::iterator(const unique_ptr<node>& rt, const node* p)
    : root(rt.get()), ptr(p) {
  assert(rt);
}

iterator& iterator::operator++() {
  if (!ptr) return *this;
  ptr = ptr->children.empty() ? ptr->next_node() : ptr->first_key();
  return *this;
}

iterator iterator::operator++(int) {
  auto temp(*this);
  ++(*this);
  return temp;
}

iterator& iterator::operator--() {
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

iterator iterator::operator--(int) {
  auto temp(*this);
  --(*this);
  return temp;
}

string iterator::operator*() const { return ptr->underlying_string(); }

iterator::operator bool() const { return ptr != nullptr; }

std::string iterator::to_json(bool include_ends) const {
  return ptr ? ptr->to_json(include_ends) : "{}";
}
