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
using std::runtime_error;
using std::string;
using std::swap;
using std::unique_ptr;

trie::trie() : root(make_unique<node>(false, nullptr)) {
  root->assert_invariants();
}

trie::trie(const initializer_list<string>& key_list)
    : trie(key_list.begin(), key_list.end()) {}

trie::trie(const trie& other) : trie(other.root->clone()) {}

trie& trie::operator=(trie other) {
  swap(root, other.root);
  root->assert_invariants();
  return *this;
}

trie::trie(unique_ptr<node>&& cloned) {
  swap(root, cloned);
  root->assert_invariants();
}

bool trie::empty(string prefix) const {
  const auto prf_rt = root->prefix_match(prefix);
  // Check if prefix root is null
  if (!prf_rt) return true;
  // It's empty if prf_rt is not a word and has no children.
  return !prf_rt->is_end && prf_rt->children.empty();
}

size_t trie::size(string prefix) const {
  const auto prf_rt = root->prefix_match(prefix);
  return prf_rt ? prf_rt->key_count() : size_t(0);
}

iterator trie::find(const string& key) const {
  // Handle edge case of key being empty.
  if (key.empty()) {
    return root->is_end ? iterator(root, root.get()) : iterator(root, nullptr);
  }
  return iterator(root, root->exact_match(key));
}

iterator trie::find_prefix(string prefix) const {
  // We need only find a word that key is a prefix of.
  const auto prf_rt = root->prefix_match(prefix);
  // If key is not a prefix of anything, there is no match.
  if (!prf_rt) return iterator(root, nullptr);

  // Find the first child key rooted at prt_rt.
  // If key is empty and prf_rt is and end node, then it is the "first key".
  return prefix.empty() && prf_rt->is_end ? iterator(root, prf_rt)
                                          : iterator(root, prf_rt->first_key());
}

iterator trie::insert(string key) {
  /*
  Note: inserting key at root, is the same
  as inserting reduced key at loc.
  The problem space has been reduced.
  */
  auto loc = const_cast<node*>(root->approximate_match(key));
  assert(loc);
  /* INSERT KEY AT LOC */

  // If the key is now empty, simply set is_end to true.
  if (key.empty()) {
    loc->is_end = true;
    root->assert_invariants();
    return iterator(root, loc);
  }

  /*
  At this point, the key is non-empty.
  If loc has no children, then just make a child.
  */
  if (loc->children.empty()) {
    {
      auto child = make_unique<node>(true, loc);
      loc->children.emplace(key, std::move(child));
    }
    root->assert_invariants();
    return iterator(root, loc->children[key].get());
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
      auto junction = make_unique<node>(post_key.empty(), loc);
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
        auto key_node = make_unique<node>(true, junction.get());
        junction->children.emplace(post_key, std::move(key_node));
      }
      root->assert_invariants();
      return iterator(root, junction->children[post_key].get());
    } else {
      root->assert_invariants();
      return iterator(root, junction.get());
    }
  }

  // If there are no shared prefixes, then simply create a node under loc.
  {
    auto key_node = make_unique<node>(true, loc);
    loc->children.emplace(key, std::move(key_node));
  }
  root->assert_invariants();
  return iterator(root, loc->children[key].get());
}

void trie::erase(string key) {
  // Must remove exact key.
  const auto match = const_cast<node*>(root->exact_match(key));
  // If the key was not in the tree, just return.
  if (!match) return;
  // No matter what happens, setting is_end to false is correct.
  match->is_end = false;

  // If match is the root node, it won't have a parent to deal with.
  if (match == root.get()) {
    // If key was non-empty, exact_match failed.
    assert(key.empty());
    root->assert_invariants();
    return;
  }

  if (match->children.empty()) {
    const auto par = const_cast<node*>(match->parent);
    auto match_iter = par->find_child(match);
    par->children.erase(match_iter);

    // Check for possible joining with grand parent.
    if (par->children.size() == 1 && par != root.get() && !par->is_end) {
      const auto grand_par = const_cast<node*>(par->parent);
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
    const auto par = const_cast<node*>(match->parent);
    assert(par);
    const auto match_iter = par->find_child(match);
    string joined_key = match_iter->first + only_child->first;

    only_child->second->parent = par;
    par->children.emplace(joined_key, std::move(only_child->second));
    par->children.erase(match_iter);
  }

  root->assert_invariants();
  // If match has multiple children, nothing can be joined.
}

void trie::erase_prefix(string prefix) {
  auto prf_ptr = root->prefix_match(prefix);
  if (!prf_ptr) return;
  if (prf_ptr == root.get()) {
    clear();
  } else {
    const auto par = const_cast<node*>(prf_ptr->parent);
    assert(par);
    par->children.erase(par->find_child(prf_ptr));
  }
  root->assert_invariants();
}

void trie::clear() {
  // Clear everything under root.
  root->children.clear();
  root->is_end = false;
  assert(!root->parent);
  root->assert_invariants();
}

string trie::to_json(bool include_ends) const {
  return root->to_json(include_ends);
}

iterator trie::begin() const {
  return root->is_end ? iterator(root, root.get())
                      : iterator(root, root->first_key());
}

iterator trie::end() const { return iterator(root, nullptr); }

iterator trie::begin(const string& prefix) const {
  // Find the first key that matches the given prefix.
  return find_prefix(prefix);
}

iterator trie::end(string prefix) const {
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
    return iterator(root, app_ptr->next_node());

  // Find the first child that is greater than prefix
  for (auto& str_ptr_pair : app_ptr->children) {
    // If equality, then approximate_match failed.
    assert(str_ptr_pair.first != prefix);
    if (str_ptr_pair.first.front() > prefix.front()) {
      return str_ptr_pair.second->is_end
                 ? iterator(root, str_ptr_pair.second.get())
                 : iterator(root, str_ptr_pair.second->first_key());
    }
  }

  // If we've gotten down to here, something has gone wrong.
  throw runtime_error("Unexpected bug in Trie::end(string)");
}

trie& trie::operator+=(const trie& rhs) {
  assert(this != &rhs);
  for (const string& key : rhs) {
    insert(key);
  }
  root->assert_invariants();
  return *this;
}

trie operator+(trie lhs, const trie& rhs) { return lhs += rhs; }

trie& trie::operator-=(const trie& rhs) {
  assert(this != &rhs);
  for (const string& key : rhs) {
    erase(key);
  }
  root->assert_invariants();
  return *this;
}

trie operator-(trie lhs, const trie& rhs) { return lhs -= rhs; }

bool operator==(const trie& lhs, const trie& rhs) {
  return lhs.root->equals(rhs.root.get());
}

bool operator!=(const trie& lhs, const trie& rhs) { return !(lhs == rhs); }

bool operator<(const trie& lhs, const trie& rhs) {
  if (lhs.size() >= rhs.size()) return false;
  return includes(rhs.begin(), rhs.end(), lhs.begin(), lhs.end());
}

bool operator>(const trie& lhs, const trie& rhs) { return rhs < lhs; }

bool operator<=(const trie& lhs, const trie& rhs) { return !(rhs < lhs); }

bool operator>=(const trie& lhs, const trie& rhs) { return !(lhs < rhs); }

bool operator==(const iterator& lhs, const iterator& rhs) {
  // Performs element by element.
  return lhs.ptr == rhs.ptr;
}

bool operator!=(const iterator& lhs, const iterator& rhs) {
  // Performs element by element.
  return lhs.ptr != rhs.ptr;
}
