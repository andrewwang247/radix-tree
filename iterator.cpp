/*
Copyright 2026. Andrew Wang.

Implementation for Trie iterator.
*/
#include "iterator.h"

#include <stdexcept>
#include <string>

using std::runtime_error;
using std::string;

iterator::iterator(const node* p) : ptr(p) {}

iterator& iterator::operator++() {
  /*
  If the ptr has children, return the first child.
  Otherwise, return the next node that isn't a child.
  Elegantly handles the case when the returned value is nullptr.
  */
  ptr = ptr->children.empty() ? ptr->next_node() : ptr->first_key();
  return *this;
}

iterator iterator::operator++(int) {
  auto temp(*this);
  ++(*this);
  return temp;
}

string iterator::operator*() const { return ptr->underlying_string(); }

iterator::operator bool() const { return ptr != nullptr; }
