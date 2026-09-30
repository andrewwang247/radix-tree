/*
Compressed radix tree.

Copyright 2026. Andrew Wang.
*/
#include "trie.h"

#include <algorithm>
#include <cassert>
#include <compare>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>

#include "iterator.h"
#include "lexi.h"
#include "node.h"

using std::initializer_list;
using std::make_unique;
using std::partial_ordering;
using std::size_t;
using std::string;
using std::string_view;
using std::strong_ordering;

namespace ranges = std::ranges;

namespace rt {

trie::trie() : root{make_unique<node>(false, nullptr)} {}

trie::trie(const trie& other) : root{other.root->clone()} {}

trie& trie::operator=(trie other) {
  std::swap(root, other.root);
  return *this;
}

trie::trie(const initializer_list<string_view>& key_list) : trie{} {
  insert_range(key_list);
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

iterator trie::begin() const noexcept {
  return root->is_end ? iterator{root, root}
                      : iterator{root, root->first_key()};
}

iterator trie::end() const noexcept { return {root, nullptr}; }

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

ranges::subrange<iterator> trie::find_prefix(
    string_view prefix) const noexcept {
  const auto left = find_prefix_begin(prefix);
  if (left == end()) {
    return {};
  }
  return {left, find_prefix_end(prefix)};
}

iterator trie::find_prefix_begin(string_view prefix) const noexcept {
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

iterator trie::find_prefix_end(string_view prefix) const noexcept {
  // Perform an approximate match.
  auto [prf_pos, app_ptr] = root->approximate_match(prefix);

  // If prefix is empty, app_ptr is a prefix match and
  // none of its children work. If all children of app_ptr
  // are less than prefix, nothing under app_ptr works.
  if (prf_pos.empty() || app_ptr->children.empty() ||
      app_ptr->children.rbegin()->first < prf_pos)
    return {root, app_ptr->next_node()};

  // Find the first child that is outside of prefix range.
  const auto next_pos = lexicographic::increment(prf_pos);
  const auto it = app_ptr->children.lower_bound(next_pos);

  // If none found, return end iterator.
  if (it == app_ptr->children.end()) {
    return {root, nullptr};
  }

  const auto& ptr = it->second;
  return ptr->is_end ? iterator{root, ptr} : iterator{root, ptr->first_key()};
}

iterator trie::insert(string_view key) {
  // Note: inserting key at root, is the same
  // as inserting reduced key at loc.
  // The problem space has been reduced.
  const auto [key_pos, loc] = root->approximate_match(key);

  // INSERT KEY AT LOC

  // If the key is now empty, simply set is_end to true.
  if (key_pos.empty()) {
    loc->is_end = true;
    return {root, loc};
  }

  // Check children of loc for shared prefixes.
  auto loc_it = loc->children.lower_bound(key_pos.substr(0, 1));

  // If there are no shared prefixes, then simply create a node under loc.
  if (loc_it == loc->children.end() ||
      !loc_it->first.starts_with(key_pos.front())) {
    const auto [key_iter, _] =
        loc->children.emplace(key_pos, make_unique<node>(true, loc));
    return {root, key_iter->second};
  }

  // Store local copy of child_str due to upcoming invalidation.
  const auto child_str = loc_it->first;
  // Use mismatch to compute the spot where the prefix fails.
  const auto [key_it, child_it] = ranges::mismatch(key_pos, child_str);
  // Extract the common prefix and unique postfixes of key and child.
  const auto common = string_view{key_pos.begin(), key_it};
  const auto post_key = string_view{key_it, key_pos.end()};
  const auto post_child = string_view{child_it, child_str.end()};
  // If key_pos prefix matches a child, approximate_match failed.
  assert(!post_child.empty());

  // Create a child for the common part. Invalidates loc_it.
  const auto [common_iter, _1] =
      loc->children.emplace(common, make_unique<node>(post_key.empty(), loc));
  const auto& junction = common_iter->second;

  // Re-assign to loc_it using child_str. No longer invalid!
  loc_it = loc->children.find(child_str);
  assert(loc_it != loc->children.end());

  // Child postfix is added to junction's children map.
  loc_it->second->parent = junction.get();
  junction->children.emplace(post_child, std::move(loc_it->second));
  // loc_it's unique_ptr has been moved. Remove it from map.
  loc->children.erase(loc_it);

  if (!post_key.empty()) {
    // Add an additional node for the split.
    const auto [junction_iter, _2] = junction->children.emplace(
        post_key, make_unique<node>(true, junction.get()));
    return {root, junction_iter->second};
  }

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
    return;
  }

  auto* par = match->parent;
  assert(par);
  auto match_iter = par->find_child(match);

  if (match->children.empty()) {
    // Invalidates match and match_iter
    par->children.erase(match_iter);

    // Check for possible joining with grand parent.
    if (par->children.size() == 1 && par != root.get() && !par->is_end) {
      auto* grand_par = par->parent;
      assert(grand_par);
      auto par_iter = grand_par->find_child(par);
      assert(par_iter != grand_par->children.end());

      // Store local copy of par_str due to upcoming invalidation.
      // Modifying grand_par affects both par and match levels.
      const auto par_str = par_iter->first;

      // Join keys on par_iter and the only child of par.
      const auto mod_key = par_str + par->children.begin()->first;
      auto& child = par->children.begin()->second;
      child->parent = grand_par;

      // Invalidates par, par_iter, and child.
      grand_par->children.emplace(mod_key, std::move(child));
      grand_par->children.erase(par_str);
    }
  } else if (match->children.size() == 1) {
    // Store local copy of match_str due to upcoming invalidation.
    const auto match_str = match_iter->first;

    // Extract child and parent string to form joined key.
    const auto only_child = match->children.begin();
    const auto joined_key = match_str + only_child->first;
    only_child->second->parent = par;

    // Invalidates match, match_iter, and only_child.
    par->children.emplace(joined_key, std::move(only_child->second));
    par->children.erase(match_str);
  }

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
}

void trie::clear() noexcept {
  // Clear everything under root.
  root->children.clear();
  root->is_end = false;
  assert(!root->parent);
}

string trie::to_json(bool include_ends) const {
  return root->to_json(include_ends);
}

void trie::assert_invariants() const noexcept { root->assert_invariants(); }

trie& trie::operator+=(const trie& other) {
  if (this == &other) return *this;
  insert_range(other);
  return *this;
}

trie operator+(trie lhs, const trie& rhs) { return lhs += rhs; }

trie& trie::operator-=(const trie& other) {
  if (this == &other) {
    clear();
    return *this;
  }
  erase_range(other);
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

}  // namespace rt
