/*
Copyright 2026. Andrew Wang.

Implementation for Trie.
*/
#include "trie.h"

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

using std::includes;
using std::initializer_list;
using std::make_unique;
using std::ostream;
using std::runtime_error;
using std::string;
using std::swap;

Trie::Trie() : root(make_unique<Node>(false, nullptr)) {
  assert(root->check_invariant());
}

Trie::Trie(const initializer_list<string>& key_list) : Trie() {
  for (const auto& key : key_list) {
    insert(key);
  }
  assert(root->check_invariant());
}

Trie::Trie(const Trie& other) : Trie() {
  assert(other.root);
  root->copy_from(other.root.get());
  assert(root->check_invariant());
}

Trie::Trie(Trie&& other) : Trie() {
  swap(*this, other);
  assert(root->check_invariant());
}

Trie& Trie::operator=(Trie other) {
  swap(*this, other);
  assert(root->check_invariant());
  return *this;
}

bool Trie::empty(string prefix) const {
  const auto prf_rt = root->prefix_match(prefix);
  // Check if prefix root is null
  if (!prf_rt) return true;
  // It's empty if prf_rt is not a word and has no children.
  assert(root->check_invariant());
  return !prf_rt->is_end && prf_rt->children.empty();
}

size_t Trie::size(string prefix) const {
  const auto prf_rt = root->prefix_match(prefix);
  return prf_rt ? prf_rt->key_count() : size_t(0);
}

Trie::iterator Trie::find(const string& key) const {
  return iterator(root->exact_match(key));
}

Trie::iterator Trie::find_prefix(string prefix) const {
  // W need only find a word that key is a prefix of.
  const auto prf_rt = root->prefix_match(prefix);
  // If key is not a prefix of anything, there is no match.
  if (!prf_rt) return iterator();

  // Find the first child key rooted at prt_rt.
  assert(root->check_invariant());
  // If key is empty and prf_rt is and end node, then it is the "first key".
  return prefix.empty() && prf_rt->is_end ? iterator(prf_rt)
                                          : iterator(prf_rt->first_key());
}

Trie::iterator Trie::insert(string key) {
  /*
  Note: inserting key at root, is the same
  as inserting reduced key at loc.
  The problem space has been reduced.
  */
  auto loc = const_cast<Node*>(root->approximate_match(key));
  assert(loc);
  /* INSERT KEY AT LOC */

  // If the key is now empty, simply set is_end to true.
  if (key.empty()) {
    loc->is_end = true;
    assert(root->check_invariant());
    return iterator(loc);
  }

  /*
  At this point, the key is non-empty.
  If loc has no children, then just make a child.
  */
  if (loc->children.empty()) {
    {
      auto child = make_unique<Node>(true, loc);
      loc->children.emplace(key, std::move(child));
    }
    assert(root->check_invariant());
    return iterator(loc->children[key].get());
  }

  // Check children of loc for shared prefixes.
  for (auto& str_ptr_pair : loc->children) {
    const string& child_str = str_ptr_pair.first;
    assert(!child_str.empty());

    // Keep iterating until a first letter match is found.
    if (child_str.front() != key.front()) continue;

    // Use mismatch to compute the spot where the prefix fails.
    auto iter_pair = mismatch(key.begin(), key.end(), child_str.begin());
    // Extract the common prefix and unique postfixes of key and child.
    string common(key.begin(), iter_pair.first);
    string post_key(iter_pair.first, key.end());
    string post_child(iter_pair.second, child_str.end());
    /*
    If remaining key's prefix can match a child,
    then approximate_match failed.
    */
    assert(!post_child.empty());

    {
      // Create a child for the common part. junction's parent is set.
      auto junction = make_unique<Node>(post_key.empty(), loc);
      // Add junction to loc under common.
      loc->children.emplace(common, std::move(junction));
    }
    const auto& junction = loc->children[common];

    // loc child is added to junction's children map.
    junction->children.emplace(post_child, std::move(str_ptr_pair.second));
    // The original child's parent pointer is set to junction.
    junction->children[post_child]->parent = junction.get();
    // Remove child_str from loc child map, cleaning up released
    // str_ptr_pair.second.
    loc->children.erase(child_str);

    if (!post_key.empty()) {
      // Add an additional node for the split.
      {
        auto key_node = make_unique<Node>(true, junction.get());
        junction->children.emplace(post_key, std::move(key_node));
      }
      assert(root->check_invariant());
      return iterator(junction->children[post_key].get());
    } else {
      assert(root->check_invariant());
      return iterator(junction.get());
    }
  }

  // If there are no shared prefixes, then simply create a node under loc.
  {
    auto key_node = make_unique<Node>(true, loc);
    loc->children.emplace(key, std::move(key_node));
  }
  assert(root->check_invariant());
  return iterator(loc->children[key].get());
}

void Trie::erase(string key) {
  // Must remove exact key.
  const auto match = const_cast<Node*>(root->exact_match(key));
  // If the key was not in the tree, just return.
  if (!match) return;
  // No matter what happens, setting is_end to false is correct.
  match->is_end = false;

  // If match is the root node, it won't have a parent to deal with.
  if (match == root.get()) {
    // If key was non-empty, exact_match failed.
    assert(key.empty());
    assert(root->check_invariant());
    return;
  }

  if (match->children.empty()) {
    const auto par = const_cast<Node*>(match->parent);
    auto match_iter = par->find_child(match);
    par->children.erase(match_iter);

    // Check for possible joining with grand parent.
    if (par->children.size() == 1 && par != root.get() && !par->is_end) {
      const auto grand_par = const_cast<Node*>(par->parent);
      assert(grand_par);
      auto par_iter = grand_par->find_child(par);
      assert(par_iter != grand_par->children.end());

      // Join keys on par_iter and the only child of par.
      string mod_key = par_iter->first + par->children.begin()->first;
      {
        auto& child = par->children.begin()->second;
        grand_par->children.emplace(mod_key, std::move(child));
      }
      grand_par->children[mod_key]->parent = grand_par;
      grand_par->children.erase(par_iter);
    }
  } else if (match->children.size() == 1) {
    // Extract child and parent string to form joined key.
    const auto only_child = match->children.begin();
    const auto par = const_cast<Node*>(match->parent);
    assert(par);
    const auto match_iter = par->find_child(match);
    string joined_key = match_iter->first + only_child->first;

    only_child->second->parent = par;
    par->children.emplace(joined_key, std::move(only_child->second));
    par->children.erase(match_iter);
  }

  assert(root->check_invariant());
  // If match has multiple children, nothing can be joined.
}

void Trie::erase_prefix(string prefix) {
  auto prf_ptr = root->prefix_match(prefix);
  if (!prf_ptr) return;
  if (prf_ptr == root.get()) {
    clear();
  } else {
    const auto par = const_cast<Node*>(prf_ptr->parent);
    assert(par);
    par->children.erase(par->find_child(prf_ptr));
  }
  assert(root->check_invariant());
}

void Trie::clear() {
  // Clear everything under root.
  root->children.clear();
  root->is_end = false;
  assert(!root->parent);
  assert(root->check_invariant());
}

Trie::iterator::iterator(const Node* p) : ptr(p) {}

Trie::iterator& Trie::iterator::operator++() {
  /*
  If the ptr has children, return the first child.
  Otherwise, return the next node that isn't a child.
  Elegantly handles the case when the returned value is nullptr.
  */
  ptr = ptr->children.empty() ? ptr->next_node() : ptr->first_key();
  return *this;
}

Trie::iterator Trie::iterator::operator++(int) {
  auto temp(*this);
  ++(*this);
  return temp;
}

string Trie::iterator::operator*() const { return ptr->underlying_string(); }

Trie::iterator::operator bool() const { return ptr != nullptr; }

Trie::iterator Trie::begin() const {
  return root->is_end ? iterator(root.get()) : iterator(root->first_key());
}

Trie::iterator Trie::end() const { return iterator(nullptr); }

Trie::iterator Trie::begin(const string& prefix) const {
  // Find the first key that matches the given prefix.
  return find_prefix(prefix);
}

Trie::iterator Trie::end(string prefix) const {
  // Perform an approximate match.
  auto app_ptr = root->approximate_match(prefix);
  assert(app_ptr);

  /*
  If prefix is empty, app_ptr is a prefix match and
  none of its children work. If all children of app_ptr
  are less than prefix, nothing under app_ptr works.
  */
  if (prefix.empty() || app_ptr->children.empty() ||
      app_ptr->children.rbegin()->first < prefix)
    return iterator(app_ptr->next_node());

  // Find the first child that is greater than prefix
  for (auto& str_ptr_pair : app_ptr->children) {
    // If equality, then approximate_match failed.
    assert(str_ptr_pair.first != prefix);
    if (str_ptr_pair.first.front() > prefix.front()) {
      return str_ptr_pair.second->is_end
                 ? iterator(str_ptr_pair.second.get())
                 : iterator(str_ptr_pair.second->first_key());
    }
  }

  // If we've gotten down to here, something has gone wrong.
  throw runtime_error("Unexpected bug in Trie::end(string)");
}

Trie& Trie::operator+=(const Trie& rhs) {
  assert(this != &rhs);
  for (const string& key : rhs) {
    insert(key);
  }
  assert(root->check_invariant());
  return *this;
}

Trie operator+(Trie lhs, const Trie& rhs) { return lhs += rhs; }

Trie& Trie::operator-=(const Trie& rhs) {
  assert(this != &rhs);
  for (const string& key : rhs) {
    erase(key);
  }
  assert(root->check_invariant());
  return *this;
}

Trie operator-(Trie lhs, const Trie& rhs) { return lhs -= rhs; }

bool operator==(const Trie& lhs, const Trie& rhs) {
  return lhs.root->equals(rhs.root.get());
}

bool operator!=(const Trie& lhs, const Trie& rhs) { return !(lhs == rhs); }

bool operator<(const Trie& lhs, const Trie& rhs) {
  if (lhs.size() >= rhs.size()) return false;
  return includes(rhs.begin(), rhs.end(), lhs.begin(), lhs.end());
}

bool operator>(const Trie& lhs, const Trie& rhs) { return rhs < lhs; }

bool operator<=(const Trie& lhs, const Trie& rhs) { return !(rhs < lhs); }

bool operator>=(const Trie& lhs, const Trie& rhs) { return !(lhs < rhs); }

ostream& operator<<(ostream& os, const Trie& tree) {
  for (const auto& str : tree) {
    os << str << '\n';
  }
  return os;
}

bool operator==(const Trie::iterator& lhs, const Trie::iterator& rhs) {
  // Performs element by element.
  return lhs.ptr == rhs.ptr;
}

bool operator!=(const Trie::iterator& lhs, const Trie::iterator& rhs) {
  // Performs element by element.
  return lhs.ptr != rhs.ptr;
}
