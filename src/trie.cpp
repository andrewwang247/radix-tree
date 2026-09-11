/*
Copyright 2026. Andrew Wang.

Implementation for Trie.
*/
#include "trie.h"

#include <algorithm>
#include <cassert>
#include <compare>
#include <cstddef>
#include <exception>
#include <initializer_list>
#include <memory>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>

#include "iterator.h"
#include "node.h"

using std::initializer_list;
using std::make_unique;
using std::partial_ordering;
using std::size_t;
using std::string;
using std::string_view;
using std::strong_ordering;
using std::terminate;
using std::unique_ptr;

namespace ranges = std::ranges;

trie::trie() : root(make_unique<node>(false, nullptr)) {
  root->assert_invariants();
}

trie::trie(const initializer_list<string_view>& key_list)
    : trie(key_list.begin(), key_list.end()) {}

trie::trie(const trie& other) : trie(other.root->clone()) {}

trie& trie::operator=(trie other) {
  std::swap(root, other.root);
  root->assert_invariants();
  return *this;
}

trie::trie(unique_ptr<node> cloned) noexcept {
  std::swap(root, cloned);
  root->assert_invariants();
}

bool trie::empty(string_view prefix) const noexcept {
  const auto [_, prf_rt] = root->prefix_match(prefix);
  // Check if prefix root is null
  if (!prf_rt) return true;
  // It's empty if prf_rt is not a word and has no children.
  return !prf_rt->is_end && prf_rt->children.empty();
}

size_t trie::size(string_view prefix) const noexcept {
  const auto [_, prf_rt] = root->prefix_match(prefix);
  return prf_rt ? prf_rt->key_count() : 0U;
}

bool trie::contains(string_view key) const noexcept {
  if (key.empty()) {
    return root->is_end;
  }
  return root->exact_match(key);
}

iterator trie::find(string_view key) const noexcept {
  // Handle edge case of key being empty.
  if (key.empty()) {
    return root->is_end ? iterator{root, root} : iterator{root, nullptr};
  }
  return {root, root->exact_match(key)};
}

iterator trie::find_prefix(string_view prefix) const noexcept {
  // We need only find a word that key is a prefix of.
  const auto [prf_pos, prf_rt] = root->prefix_match(prefix);
  // If key is not a prefix of anything, there is no match.
  if (!prf_rt) return {root, nullptr};

  // Find the first child key rooted at prt_rt.
  // If key is empty and prf_rt is and end node, then it is the "first key".
  return prf_pos.empty() && prf_rt->is_end
             ? iterator{root, prf_rt}
             : iterator{root, prf_rt->first_key()};
}

iterator trie::insert(string_view key) {
  // Note: inserting key at root, is the same
  // as inserting reduced key at loc.
  // The problem space has been reduced.
  const auto [key_pos, loc] = root->approximate_match(key);
  assert(loc);

  // INSERT KEY AT LOC

  // If the key is now empty, simply set is_end to true.
  if (key_pos.empty()) {
    loc->is_end = true;
    root->assert_invariants();
    return {root, loc};
  }

  const auto first_char_match = [key_pos](string_view sv) constexpr {
    assert(!sv.empty());
    return sv.front() == key_pos.front();
  };
  // Check children of loc for shared prefixes.
  auto loc_it = ranges::find_if(loc->children, first_char_match,
                                &decltype(loc->children)::value_type::first);

  // If there are no shared prefixes, then simply create a node under loc.
  if (loc_it == loc->children.end()) {
    const auto [key_iter, _] =
        loc->children.emplace(key_pos, make_unique<node>(true, loc));
    root->assert_invariants();
    return {root, key_iter->second};
  }

  // Use mismatch to compute the spot where the prefix fails.
  const auto child_str = string_view{loc_it->first};
  const auto [key_it, child_it] = ranges::mismatch(key_pos, child_str);
  // Extract the common prefix and unique postfixes of key and child.
  const auto common = string_view{key_pos.begin(), key_it};
  const auto post_key = string_view{key_it, key_pos.end()};
  const auto post_child = string_view{child_it, child_str.end()};
  // If key_pos prefix matches a child, approximate_match failed.
  assert(!post_child.empty());

  // Create a child for the common part.
  const auto [common_iter, _1] =
      loc->children.emplace(common, make_unique<node>(post_key.empty(), loc));
  const auto& junction = common_iter->second;

  // loc child is added to junction's children map.
  auto [post_iter, _2] =
      junction->children.emplace(post_child, std::move(loc_it->second));
  // The original child's parent pointer is set to junction.
  post_iter->second->parent = junction.get();
  // Remove child_str from loc child map, cleaning up released child_ptr.
  loc->children.erase(loc_it);

  if (!post_key.empty()) {
    // Add an additional node for the split.
    const auto [junction_iter, _3] = junction->children.emplace(
        post_key, make_unique<node>(true, junction.get()));
    root->assert_invariants();
    return {root, junction_iter->second};
  }

  root->assert_invariants();
  return {root, junction};
}

void trie::erase(string_view key) {
  // Must remove exact key.
  auto* match = root->exact_match(key);
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

  auto* par = match->parent;
  assert(par);
  auto match_iter = par->find_child(match);

  if (match->children.empty()) {
    par->children.erase(match_iter);

    // Check for possible joining with grand parent.
    if (par->children.size() == 1 && par != root.get() && !par->is_end) {
      auto* grand_par = par->parent;
      assert(grand_par);
      auto par_iter = grand_par->find_child(par);
      assert(par_iter != grand_par->children.end());

      // Join keys on par_iter and the only child of par.
      const auto mod_key = par_iter->first + par->children.begin()->first;
      auto& child = par->children.begin()->second;
      const auto [key_iter, _] =
          grand_par->children.emplace(mod_key, std::move(child));
      key_iter->second->parent = grand_par;
      grand_par->children.erase(par_iter);
    }
  } else if (match->children.size() == 1) {
    // Extract child and parent string to form joined key.
    const auto only_child = match->children.begin();
    const auto joined_key = match_iter->first + only_child->first;

    only_child->second->parent = par;
    par->children.emplace(joined_key, std::move(only_child->second));
    par->children.erase(match_iter);
  }

  root->assert_invariants();
  // If match has multiple children, nothing can be joined.
}

void trie::erase_prefix(string_view prefix) {
  const auto [prf_pos, prf_ptr] = root->prefix_match(prefix);
  if (!prf_ptr) return;
  if (prf_ptr == root.get()) {
    clear();
  } else {
    auto* par = prf_ptr->parent;
    assert(par);
    par->children.erase(par->find_child(prf_ptr));
  }
  root->assert_invariants();
}

void trie::clear() noexcept {
  // Clear everything under root.
  root->children.clear();
  root->is_end = false;
  assert(!root->parent);
  root->assert_invariants();
}

string trie::to_json(bool include_ends) const {
  return root->to_json(include_ends);
}

iterator trie::begin() const noexcept {
  return root->is_end ? iterator{root, root}
                      : iterator{root, root->first_key()};
}

iterator trie::end() const noexcept { return {root, nullptr}; }

iterator trie::begin(string_view prefix) const noexcept {
  // Find the first key that matches the given prefix.
  return find_prefix(prefix);
}

iterator trie::end(string_view prefix) const noexcept {
  // Perform an approximate match.
  auto [prf_pos, app_ptr] = root->approximate_match(prefix);
  assert(app_ptr);

  // If prefix is empty, app_ptr is a prefix match and
  // none of its children work. If all children of app_ptr
  // are less than prefix, nothing under app_ptr works.
  if (prf_pos.empty() || app_ptr->children.empty() ||
      app_ptr->children.rbegin()->first < prf_pos)
    return {root, app_ptr->next_node()};

  // Find the first child that is greater than prefix
  for (const auto& [str, ptr] : app_ptr->children) {
    // If equality, then approximate_match failed.
    assert(str != prf_pos);
    if (str.front() > prf_pos.front()) {
      return ptr->is_end ? iterator{root, ptr}
                         : iterator{root, ptr->first_key()};
    }
  }

  // If we've gotten down to here, something has gone wrong.
  terminate();
}

ranges::subrange<iterator> trie::subrange(string_view prefix) const noexcept {
  return {begin(prefix), end(prefix)};
}

trie& trie::operator+=(const trie& rhs) {
  assert(this != &rhs);
  for (const auto& key : rhs) {
    insert(key);
  }
  root->assert_invariants();
  return *this;
}

trie operator+(trie lhs, const trie& rhs) { return lhs += rhs; }

trie& trie::operator-=(const trie& rhs) {
  assert(this != &rhs);
  for (const auto& key : rhs) {
    erase(key);
  }
  root->assert_invariants();
  return *this;
}

trie operator-(trie lhs, const trie& rhs) { return lhs -= rhs; }

bool operator==(const trie& lhs, const trie& rhs) noexcept {
  return node::deep_equals(lhs.root.get(), rhs.root.get());
}

partial_ordering operator<=>(const trie& lhs, const trie& rhs) noexcept {
  auto left_it = lhs.begin();
  auto right_it = rhs.begin();
  const auto left_end = lhs.end();
  const auto right_end = rhs.end();

  auto left_has_extra = false;
  auto right_has_extra = false;

  while (left_it != left_end && right_it != right_end) {
    const auto element_compare = *left_it <=> *right_it;
    if (element_compare == strong_ordering::less) {
      left_has_extra = true;
      ++left_it;
    } else if (element_compare == strong_ordering::greater) {
      right_has_extra = true;
      ++right_it;
    } else {
      ++left_it;
      ++right_it;
    }

    if (left_has_extra && right_has_extra) return partial_ordering::unordered;
  }

  if (left_it != left_end) left_has_extra = true;
  if (right_it != right_end) right_has_extra = true;

  if (!left_has_extra && !right_has_extra) return partial_ordering::equivalent;
  if (!left_has_extra && right_has_extra) return partial_ordering::less;
  if (left_has_extra && !right_has_extra) return partial_ordering::greater;
  return partial_ordering::unordered;
}
