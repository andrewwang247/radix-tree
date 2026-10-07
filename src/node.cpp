/*
Internal trie node.

Copyright 2026. Andrew Wang.
*/
#include "node.h"

#include <algorithm>
#include <bitset>
#include <cassert>
#include <cstddef>
#include <format>
#include <limits>
#include <memory>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using std::format;
using std::make_unique;
using std::size_t;
using std::string;
using std::string_view;
using std::unique_ptr;
using std::vector;

namespace ranges = std::ranges;
namespace views = std::views;

namespace rt {

node::node(bool end, node* par) noexcept : parent(par), is_end(end) {}

unique_ptr<node> node::clone() const {
  // Null parent because we do not clone above this node.
  auto copy = make_unique<node>(is_end, nullptr);
  for (auto&& [str, ptr] : children) {
    assert(ptr);
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
  const auto get_raw = [](const auto& unq_ptr) {
    assert(unq_ptr);
    return unq_ptr.get();
  };
  return lhs->is_end == rhs->is_end &&
         ranges::equal(lhs->children.keys(), rhs->children.keys()) &&
         ranges::equal(lhs->children.values(), rhs->children.values(),
                       deep_equals, get_raw, get_raw);
}

size_t node::key_count() const noexcept {
  return ranges::fold_left(
      children.values(), is_end ? 1UZ : 0UZ,
      [](auto counter, const auto& ptr) { return counter + ptr->key_count(); });
}

node::positional node::approximate_match(string_view key) noexcept {
  // If the key is empty, return this.
  if (key.empty()) return {.pos = key, .ptr = this};

  // Since none of the children share a common prefix, we check first char.
  const auto it = children.lower_bound(key.substr(0, 1));

  // If exists a child that is a prefix of key,
  // strip child from key and recurse.
  // Otherwise, simply return this.
  const auto is_prefix = it != children.end() && key.starts_with(it->first);
  return is_prefix
             ? it->second->approximate_match(key.substr(it->first.length()))
             : positional{.pos = key, .ptr = this};
}

node::positional node::prefix_match(string_view prf) noexcept {
  // First compute the approximate root.
  const auto [prf_pos, app_ptr] = approximate_match(prf);
  // If the given prf is empty, it's a perfect match.
  if (prf_pos.empty()) return {.pos = prf_pos, .ptr = app_ptr};

  const auto it = app_ptr->children.lower_bound(prf_pos);

  // If any of the node's children have prf as prefix, return that child.
  // Otherwise, no way to make prf a prefix. Return null.
  const auto is_prefix =
      it != app_ptr->children.end() && it->first.starts_with(prf_pos);
  return is_prefix ? positional{.pos = "", .ptr = it->second.get()}
                   : positional{.pos = prf_pos, .ptr = nullptr};
}

node* node::exact_match(string_view word) noexcept {
  // First compute the approximate root.
  const auto [word_pos, app_ptr] = approximate_match(word);
  // Match if and only if we've used entire word and app_ptr is_end.
  return word_pos.empty() && app_ptr->is_end ? app_ptr : nullptr;
}

const node* node::first_child_node() const noexcept {
  assert(!children.empty());
  const auto* value = children.begin()->second.get();
  assert(value);
  return value;
}

const node* node::last_child_node() const noexcept {
  assert(!children.empty());
  const auto* value = children.rbegin()->second.get();
  assert(value);
  return value;
}

// --- END CHILD HELPER FUNCTIONS ---

const node* node::first_key() const noexcept {
  if (children.empty()) return nullptr;
  const auto* rt = first_child_node();
  // Keep moving down the tree along the left side until is_end.
  while (!rt->is_end) {
    // If rt is not an end, its children should not be empty.
    rt = rt->first_child_node();
  }
  return rt;
}

const node* node::last_key() const noexcept {
  if (children.empty()) return nullptr;
  const auto* rt = last_child_node();
  // Keep moving down the tree along the right side until no children.
  while (!rt->children.empty()) {
    rt = rt->last_child_node();
  }
  assert(rt->is_end);
  return rt;
}

const node* node::next_node() const noexcept {
  // Go up until we can move right.
  const auto* ptr = this;
  auto* par = parent;
  // Note that par->children cannot be empty since its a parent.
  assert(!par || !par->children.empty());
  while (par && par->last_child_node() == ptr) {
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
  while (par && !par->is_end && par->first_child_node() == ptr) {
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
  if (par->first_child_node() != ptr) {
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
    const auto* par = ptr->parent;
    // We must be able to find ptr in par->children.
    auto iter = par->find_child(ptr);
    assert(iter != par->children.end());

    // Push the string representation onto the stack.
    const auto& str_rep = iter->first;
    history.emplace_back(str_rep);
    len_sum += str_rep.size();
  }

  // If par is null, then ptr must be root. Concatenate strings in reverse.
  string out;
  out.reserve(len_sum);
  for (auto part : history | views::reverse) {
    out += part;
  }
  return out;
}

string node::to_json(bool include_ends) const {
  string header = "{";
  if (include_ends) {
    header += format(R"("end":{},"children":{{)", is_end ? "true" : "false");
  }
  const auto content =
      children | views::transform([include_ends](auto&& entry) {
        auto&& [str, ptr] = entry;
        assert(ptr);
        return format(R"("{}":{})", str, ptr->to_json(include_ends));
      }) |
      views::join_with(',') | ranges::to<string>();
  return header + content + (include_ends ? "}}" : "}");
}

void node::assert_invariants() const noexcept {
  constexpr auto max_possible_chars =
      static_cast<size_t>(std::numeric_limits<unsigned char>::max()) + 1;
  std::bitset<max_possible_chars> seen;
  for (auto&& [str, ptr] : children) {
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
}

}  // namespace rt
