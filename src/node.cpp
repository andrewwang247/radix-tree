/*
Copyright 2026. Andrew Wang.

Implementation for Node.
*/
#include "node.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <format>
#include <iterator>
#include <map>
#include <memory>
#include <ranges>
#include <string>
#include <string_view>
#include <unordered_set>  // NOLINT(misc-include-cleaner)
#include <utility>
#include <vector>

using std::format;
using std::make_unique;
using std::map;
using std::string;
using std::string_view;
using std::unique_ptr;
using std::vector;

namespace ranges = std::ranges;
namespace views = std::views;

node::node(bool end, node* par) : is_end(end), parent(par) {}

unique_ptr<node> node::clone() const {
  // Null parent because we do not clone above this node.
  auto copy = make_unique<node>(is_end, nullptr);
  for (const auto& [str, ptr] : children) {
    auto child_clone = ptr->clone();
    // Manually set child's parent to the copy.
    child_clone->parent = copy.get();
    copy->children.emplace(str, std::move(child_clone));
  }
  return copy;
}

bool node::deep_equals(const node* lhs, const node* rhs) {
  assert(lhs);
  assert(rhs);
  if (lhs->is_end != rhs->is_end) return false;
  if (lhs->children.size() != rhs->children.size()) return false;

  // Since the number of children match, we can iterate in parallel.
  for (auto left_it = lhs->children.begin(), right_it = rhs->children.begin();
       left_it != lhs->children.end() && right_it != rhs->children.end();
       ++left_it, ++right_it) {
    if (left_it->first != right_it->first) return false;

    // Recursively check for equality via depth first search.
    if (!deep_equals(left_it->second.get(), right_it->second.get()))
      return false;
  }

  return true;
}

size_t node::key_count() const {
  // If is_end, count it as a word.
  size_t counter = is_end ? 1 : 0;
  // Recursively check for words in children
  for (const auto& [_, ptr] : children) {
    assert(ptr);
    counter += ptr->key_count();
  }
  return counter;
}

node::positional node::approximate_match(string_view key_view) {
  // If the key is empty, return this.
  if (key_view.empty()) return {.ptr = this, .pos = key_view};

  for (const auto& [str, ptr] : children) {
    assert(ptr);
    // If one of the children is a prefix of key, recurse.
    if (key_view.starts_with(str)) {
      // Remove the child string off the front of key.
      return ptr->approximate_match(key_view.substr(str.length()));
    }
  }

  // If none of the children form a prefix for key, simply return this.
  return {.ptr = this, .pos = key_view};
}

node::positional node::prefix_match(string_view prf_view) {
  // First compute the approximate root.
  const auto [app_ptr, prf] = approximate_match(prf_view);
  assert(app_ptr);
  // If the given prf is empty, it's a perfect match.
  if (prf.empty()) return {.ptr = app_ptr, .pos = prf};

  // If any of the node's children have prf as prefix, return that child.
  for (const auto& [str, ptr] : app_ptr->children) {
    assert(ptr);
    if (str.starts_with(prf)) {
      return {.ptr = ptr.get(), .pos = string_view{}};
    }
  }

  // No way to make prf a prefix. Return null.
  return {.ptr = nullptr, .pos = prf};
}

node* node::exact_match(string_view word_view) {
  // First compute the approximate root.
  const auto [app_ptr, word] = approximate_match(word_view);
  assert(app_ptr);
  // If the word is empty, it's a perfect match. Otherwise, no match.
  return word.empty() ? app_ptr : nullptr;
}

const node* node::first_key() const {
  if (children.empty()) return nullptr;
  const auto* rt = this;
  // Keep moving down the tree along the left side until is_end.
  do {
    // If rt is not an end, its children should not be empty.
    assert(!rt->children.empty());
    rt = rt->children.begin()->second.get();
    assert(rt);
  } while (!rt->is_end);
  return rt;
}

const node* node::last_key() const {
  if (children.empty()) return nullptr;
  const auto* rt = this;
  // Keep moving down the tree along the right side until no children.
  do {
    rt = rt->children.rbegin()->second.get();
    assert(rt);
  } while (!rt->children.empty());
  assert(rt->is_end);
  return rt;
}

const node* node::next_node() const {
  // Go up until we can move right.
  const auto* ptr = this;
  auto* par = parent;
  // Note that par->children cannot be empty since its a parent.
  assert(!par->children.empty());
  while (par && par->children.rbegin()->second.get() == ptr) {
    // Move up.
    ptr = par;
    par = par->parent;
  }

  // If par is null, there is nothing to the right. Return null
  if (!par) return nullptr;

  // If par is non-null, the only way we broke out of the while
  // loop is because ptr is not the right-most child.
  // Thus, we want to find the child to the right of ptr.
  auto child_iter = par->find_child(ptr);
  assert(child_iter != par->children.end());
  ++child_iter;
  assert(child_iter != par->children.end());
  const auto& rn = child_iter->second;
  assert(rn);

  // Return the smallest key rooted at rn.
  // If rn is an end node, it's smaller than its children.
  if (rn->is_end) {
    return rn.get();
  }
  assert(!rn->children.empty());
  return rn->first_key();
}

const node* node::prev_node() const {
  // Go up until is_end or we can move left.
  const auto* ptr = this;
  auto* par = parent;
  // Note that par->children cannot be empty since its a parent.
  assert(!par->children.empty());
  while (par && !par->is_end && par->children.begin()->second.get() == ptr) {
    // Move up.
    ptr = par;
    par = par->parent;
  }

  // If par is null, there is nothing to the left. Return null
  if (!par) return nullptr;

  // If par is non-null, the only way we broke out of the while is:
  // 1. par has a children to the left.
  // 2. par is an end node and forms a word.
  // Case (1) takes precedence.
  // Any of par's children are more immediately prev.
  if (par->children.begin()->second.get() != ptr) {
    auto child_iter = par->find_child(ptr);
    assert(child_iter != par->children.end());
    --child_iter;
    const auto& rn = child_iter->second;
    assert(rn);

    // Return the largest key rooted at rn.
    // All children of rn are larger than it.
    if (rn->children.empty()) {
      assert(rn->is_end);
      return rn.get();
    }
    return rn->last_key();
  }

  // par has no children to the left and is an end.
  return par;
}

string node::underlying_string() const {
  vector<string_view> history;
  size_t total_length = 0;

  // Move up in trie until we get to root.
  for (const auto* ptr = this; ptr->parent; ptr = ptr->parent) {
    auto* const par = ptr->parent;
    // We must be able to find ptr in par->children.
    auto iter = par->find_child(ptr);
    assert(iter != par->children.end());

    // Push the string representation onto the stack.
    history.emplace_back(iter->first);
    total_length += iter->first.size();
  }

  // If par is null, then ptr must be root. Concatenate strings in reverse.
  string str{};
  str.reserve(total_length);
  for (const auto& segment : history | views::reverse) {
    str += segment;
  }
  return str;
}

map<string, unique_ptr<node>>::const_iterator node::find_child(
    const node* other) const {
  return ranges::find(children, other,
                      [](const auto& p) { return p.second.get(); });
}

string node::to_json(bool include_ends) const {
  string builder = include_ends ? R"({"end":)" : "{";
  if (include_ends) {
    builder += format(R"({},"children":{{)", is_end ? "true" : "false");
  }
  if (!children.empty()) {
    for (const auto& [str, ptr] : children | views::take(children.size() - 1)) {
      builder += format(R"("{}":{},)", str, ptr->to_json(include_ends));
    }
    const auto& [str, ptr] = *std::prev(children.end());
    builder += format(R"("{}":{})", str, ptr->to_json(include_ends));
  }
  builder += include_ends ? "}}" : "}";
  return builder;
}

void node::assert_invariants() const {
#ifdef DEBUG
  std::unordered_set<char> characters;
  characters.reserve(children.size());
  for (const auto& [str, ptr] : children) {
    assert(ptr);
    assert(ptr->parent == this);
    assert(!str.empty());
    // Check that string does not share a prefix with other children.
    // We only really need to check first char.
    auto [_, was_inserted] = characters.emplace(str.front());
    assert(was_inserted);
    // Recursively check child nodes.
    ptr->assert_invariants();
  }
#endif
}
