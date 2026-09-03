/*
Copyright 2026. Andrew Wang.

Implementation for Node.
*/
#include "node.h"

#include <algorithm>
#ifdef DEBUG
#include <bitset>
#endif
#include <cassert>
#include <cstddef>
#include <format>
#include <iterator>
#include <map>
#include <memory>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using std::format;
using std::make_unique;
using std::map;
using std::size_t;
using std::string;
using std::string_view;
using std::unique_ptr;
using std::vector;

namespace ranges = std::ranges;
namespace views = std::views;

node::node(bool end, node* par) noexcept : parent(par), is_end(end) {}

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

bool node::deep_equals(const node* lhs, const node* rhs) noexcept {
  assert(lhs);
  assert(rhs);
  const auto map_eq = [](const auto& lp, const auto& rp) static {
    return lp.first == rp.first &&
           deep_equals(lp.second.get(), rp.second.get());
  };
  return lhs->is_end == rhs->is_end &&  // cppcheck-suppress duplicateBreak
         ranges::equal(lhs->children, rhs->children, map_eq);
}

size_t node::key_count() const noexcept {
  // If is_end, count it as a word.
  auto counter = is_end ? 1UZ : 0UZ;
  // Recursively check for words in children
  for (const auto& [_, ptr] : children) {
    assert(ptr);
    counter += ptr->key_count();
  }
  return counter;
}

node::positional node::approximate_match(string_view key) noexcept {
  // If the key is empty, return this.
  if (key.empty()) return {.pos = key, .ptr = this};

  for (const auto& [str, ptr] : children) {
    assert(ptr);
    // If one of the children is a prefix of key, recurse.
    if (key.starts_with(str)) {
      // Remove the child string off the front of key.
      return ptr->approximate_match(key.substr(str.length()));
    }
  }

  // If none of the children form a prefix for key, simply return this.
  return {.pos = key, .ptr = this};
}

node::positional node::prefix_match(string_view prf) noexcept {
  // First compute the approximate root.
  const auto [prf_pos, app_ptr] = approximate_match(prf);
  assert(app_ptr);
  // If the given prf is empty, it's a perfect match.
  if (prf_pos.empty()) return {.pos = prf_pos, .ptr = app_ptr};

  // If any of the node's children have prf as prefix, return that child.
  for (const auto& [str, ptr] : app_ptr->children) {
    assert(ptr);
    if (str.starts_with(prf_pos)) {
      return {.pos = string_view{}, .ptr = ptr.get()};
    }
  }

  // No way to make prf a prefix. Return null.
  return {.pos = prf_pos, .ptr = nullptr};
}

node* node::exact_match(string_view word) noexcept {
  // First compute the approximate root.
  const auto [word_pos, app_ptr] = approximate_match(word);
  assert(app_ptr);
  // Match if and only if we've used entire word and app_ptr is_end.
  if (word_pos.empty() && app_ptr->is_end) return app_ptr;
  return nullptr;
}

const node* node::first_key() const noexcept {
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

const node* node::last_key() const noexcept {
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

const node* node::next_node() const noexcept {
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

const node* node::prev_node() const noexcept {
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
  auto len_sum = 0UZ;

  // Move up in trie until we get to root.
  for (const auto* ptr = this; ptr->parent; ptr = ptr->parent) {
    auto* const par = ptr->parent;
    // We must be able to find ptr in par->children.
    auto iter = par->find_child(ptr);
    assert(iter != par->children.end());

    // Push the string representation onto the stack.
    history.emplace_back(iter->first);
    len_sum += iter->first.size();
  }

  // If par is null, then ptr must be root. Concatenate strings in reverse.
  string out;
  out.reserve(len_sum);
  for (auto part : history | views::reverse) {
    out += part;
  }
  return out;
}

map<string, unique_ptr<node>>::const_iterator node::find_child(
    const node* other) const noexcept {
  return ranges::find(children, other, [](const auto& p) static constexpr {
    return p.second.get();
  });
}

string node::to_json(bool include_ends) const {
  string header = "{";
  if (include_ends) {
    header += format(R"("end":{},"children":{{)", is_end ? "true" : "false");
  }
  const auto content =
      children | views::transform([include_ends](const auto& entry) {
        const auto& [str, ptr] = entry;
        return format(R"("{}":{})", str, ptr->to_json(include_ends));
      }) |
      views::join_with(',') | ranges::to<string>();
  return header + content + (include_ends ? "}}" : "}");
}

void node::assert_invariants() const noexcept {
#ifdef DEBUG
  constexpr auto max_possible_chars = 1 << 8;
  std::bitset<max_possible_chars> seen;
  for (const auto& [str, ptr] : children) {
    assert(ptr);
    assert(ptr->parent == this);
    assert(!str.empty());
    // Check that string does not share a prefix with other children.
    // We only really need to check first char.
    const auto idx = static_cast<unsigned char>(str.front());
    assert(!seen.test(idx));
    seen.set(idx);
    // Recursively check child nodes.
    ptr->assert_invariants();
  }
#endif
}
