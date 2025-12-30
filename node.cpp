/*
Copyright 2026. Andrew Wang.

Implementation for Node.
*/
#include "node.h"

#include <algorithm>
#include <cassert>
#include <memory>
#include <stack>
#include <string>
#include <unordered_set>
#include <utility>

using std::make_unique;
using std::mismatch;
using std::stack;
using std::string;
using std::unique_ptr;
using std::unordered_set;

Node::Node(bool is_end, const Node* parent)
    : is_end(is_end), parent(parent), children() {}

void Node::copy_from(const Node* other) {
  assert(other);
  // Make rt's is_end the same as other's is_end.
  is_end = other->is_end;
  // Recursively copy children.
  for (const auto& str_ptr_pair : other->children) {
    const auto& prf = str_ptr_pair.first;
    {
      auto child = make_unique<Node>(false, this);
      children.emplace(prf, std::move(child));
    }
    children[prf]->copy_from(str_ptr_pair.second.get());
  }
}

bool Node::equals(const Node* other) const {
  assert(other);
  // Check is_end parameters match.
  if (is_end != other->is_end) return false;
  // If neither have children, we're all good to go.
  if (children.empty() && other->children.empty()) return true;
  // Check that number of children are the same.
  if (children.size() != other->children.size()) return false;
  // Since the number of children match, we can iterate in parallel.
  auto it_1 = children.begin();
  auto it_2 = other->children.begin();
  while (it_1 != children.end()) {
    // Check that the strings on the branches match.
    if (it_1->first != it_2->first) return false;
    // Recursively check for equality.
    if (!it_1->second->equals(it_2->second.get())) return false;
    ++it_1;
    ++it_2;
  }
  return true;
}

const Node* Node::approximate_match(std::string& key) const {
  // If the key is empty, return this.
  if (key.empty()) return this;

  for (const auto& str_ptr_pair : children) {
    assert(str_ptr_pair.second);
    // If one of the children is a prefix of key, recurse.
    if (is_prefix(str_ptr_pair.first, key)) {
      // Remove the child string off the front of key.
      return str_ptr_pair.second->approximate_match(
          key.erase(0, str_ptr_pair.first.length()));
    }
  }

  // If none of the children form a prefix for key, simply return this.
  return this;
}

size_t Node::key_count() const {
  // If is_end, count it as a word.
  size_t counter = is_end ? 1 : 0;
  // Recursively check for words in children
  for (const auto& str_ptr_pair : children) {
    assert(str_ptr_pair.second);
    counter += str_ptr_pair.second->key_count();
  }
  return counter;
}

const Node* Node::prefix_match(std::string& prf) const {
  // First compute the approximate root.
  const auto app_ptr = approximate_match(prf);
  assert(app_ptr);
  // If the given prf is empty, it's a perfect match.
  if (prf.empty()) return app_ptr;

  /*
  If any of the returned node's children
  have prf as prefix, return that child.
  */
  for (const auto& str_ptr_pair : app_ptr->children) {
    assert(str_ptr_pair.second);
    if (is_prefix(prf, str_ptr_pair.first)) {
      prf.clear();
      return str_ptr_pair.second.get();
    }
  }

  // No way to make prf a prefix. Return null.
  return nullptr;
}

const Node* Node::exact_match(string word) const {
  // First compute the approximate root.
  const auto app_ptr = approximate_match(word);
  assert(app_ptr);
  /*
  If the given word is empty, it's a perfect match.
  Otherwise, there is no match.
  */
  return word.empty() ? app_ptr : nullptr;
}

const Node* Node::first_key() const {
  if (children.empty()) return nullptr;
  auto rt = this;
  // Keep moving down the tree along the left side until is_end.
  do {
    // If rt is not an end, its children should not be empty.
    assert(!rt->children.empty());
    rt = rt->children.begin()->second.get();
    assert(rt);
  } while (!rt->is_end);
  return rt;
}

const Node* Node::next_node() const {
  // Go up once then keep going up until we can move right.
  auto ptr = this;
  auto par = parent;
  // Note that par->children cannot be empty since its a parent.
  assert(!par->children.empty());
  while (par && par->children.rbegin()->second.get() == ptr) {
    // Move up.
    ptr = par;
    par = par->parent;
  }

  // If par is null, there is nothing to the right. Return null
  if (!par) return nullptr;

  /*
  If par is non-null, the only way we broke out of the while
  loop is because ptr is not the right-most child.
  Thus, we want to find the child to the right of ptr.
  */
  auto child_iter = value_find(par->children, ptr);
  assert(child_iter != par->children.end());
  ++child_iter;
  assert(child_iter != par->children.end());
  const auto& rn = child_iter->second;
  assert(rn);

  // Return the smallest key rooted at rn.
  if (rn->is_end) return rn.get();
  assert(!rn->children.empty());
  return rn->first_key();
}

string Node::underlying_string() const {
  stack<string> history;
  size_t total_length = 0;

  // Move up in trie until we get to root.
  auto ptr = this;
  auto par = parent;
  while (par) {
    // We must be able to find ptr in par->children.
    auto iter = value_find(par->children, ptr);
    assert(iter != par->children.end());

    // Push the string representation onto the stack.
    history.push(iter->first);
    total_length += iter->first.size();

    ptr = par;
    par = par->parent;
  }

  // If par is null, then ptr must be root. Concatenate strings in reverse.
  string str{};
  str.reserve(total_length);
  while (!history.empty()) {
    str += history.top();
    history.pop();
  }
  return str;
}

bool Node::check_invariant() const {
  // Check that root is non-null.
  unordered_set<char> characters;

  // Check validity of children.
  for (const auto& str_ptr_pair : children) {
    // No null nodes in children tree.
    if (!str_ptr_pair.second) return false;
    // Ensure that its parent is root.
    if (str_ptr_pair.second->parent != this) return false;
    // Make sure string is not empty.
    if (str_ptr_pair.first.empty()) return false;
    /*
    Check that string does not share a prefix with other children.
    We only really need to check first char.
    */
    if (characters.find(str_ptr_pair.first.front()) != characters.end())
      return false;
    characters.insert(str_ptr_pair.first.front());
    // Recursively check child node.
    if (!str_ptr_pair.second->check_invariant()) return false;
  }

  // If root passes every single check, the tree is valid.
  return true;
}

bool is_prefix(const string& prf, const string& word) {
  // The empty string is a prefix for every string.
  if (prf.empty()) return true;
  // Assuming non-emptiness of prf, it cannot be longer than word.
  if (prf.length() > word.length()) return false;
  // std::algorithm function that returns iterators to first mismatch.
  const auto res = mismatch(prf.begin(), prf.end(), word.begin());

  // If we reached the end of prf, it's a prefix.
  return res.first == prf.end();
}
