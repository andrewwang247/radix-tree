/*
Copyright 2026. Andrew Wang.

Implementation for Trie.
*/
#include "trie.h"

#include <algorithm>
#include <map>
#include <memory>
#include <stack>
#include <string>
#include <unordered_set>
#include <utility>

using std::includes;
using std::initializer_list;
using std::make_unique;
using std::map;
using std::ostream;
using std::runtime_error;
using std::stack;
using std::string;
using std::swap;
using std::unique_ptr;
using std::unordered_set;

Trie::Node::Node(bool is_end_in, const Node* parent_in)
    : is_end(is_end_in), parent(parent_in), children() {}

void Trie::recursive_copy(const std::unique_ptr<Node>& rt,
                          const std::unique_ptr<Node>& other) {
  assert(rt && other);
  // Make rt's is_end the same as other's is_end.
  rt->is_end = other->is_end;
  // Recursively copy children.
  for (const auto& str_ptr_pair : other->children) {
    const auto& prf = str_ptr_pair.first;
    {
      auto child = make_unique<Node>(false, rt.get());
      rt->children.emplace(prf, std::move(child));
    }
    recursive_copy(rt->children[prf], str_ptr_pair.second);
  }
}

bool Trie::is_prefix(const string& prf, const string& word) {
  // The empty string is a prefix for every string.
  if (prf.empty()) return true;
  // Assuming non-emptiness of prf, it cannot be longer than word.
  if (prf.length() > word.length()) return false;
  // std::algorithm function that returns iterators to first mismatch.
  const auto res = mismatch(prf.begin(), prf.end(), word.begin());

  // If we reached the end of prf, it's a prefix.
  return res.first == prf.end();
}

const Trie::Node* Trie::approximate_match(const Trie::Node* rt, string& key) {
  assert(rt);
  // If the key is empty, return the root node.
  if (key.empty()) return rt;

  for (const auto& str_ptr_pair : rt->children) {
    assert(str_ptr_pair.second);
    // If one of the children is a prefix of key, recurse.
    if (is_prefix(str_ptr_pair.first, key)) {
      // Remove the child string off the front of key.
      return approximate_match(str_ptr_pair.second.get(),
                               key.erase(0, str_ptr_pair.first.length()));
    }
  }

  // If none of the children form a prefix for key, simply return the root.
  return rt;
}

const Trie::Node* Trie::prefix_match(const Trie::Node* rt, string& prf) {
  // First compute the approximate root.
  const auto app_ptr = approximate_match(rt, prf);
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

void Trie::key_counter(const Node* rt, size_t& acc) {
  assert(rt);
  // If root contains a word, increment the counter.
  if (rt->is_end) ++acc;
  // Recursively check for words in children
  for (const auto& str_ptr_pair : rt->children) {
    assert(str_ptr_pair.second);
    key_counter(str_ptr_pair.second.get(), acc);
  }
}

const Trie::Node* Trie::exact_match(const Trie::Node* rt, string word) {
  // First compute the approximate root.
  const auto app_ptr = approximate_match(rt, word);
  assert(app_ptr);
  /*
  If the given word is empty, it's a perfect match.
  Otherwise, there is no match.
  */
  return word.empty() ? app_ptr : nullptr;
}

bool Trie::are_equal(const std::unique_ptr<Node>& rt_1,
                     const std::unique_ptr<Node>& rt_2) {
  assert(rt_1 && rt_2);
  // Check is_end parameters match.
  if (rt_1->is_end != rt_2->is_end) return false;
  // If neither have children, we're all good to go.
  if (rt_1->children.empty() && rt_2->children.empty()) return true;
  // Check that number of children are the same.
  if (rt_1->children.size() != rt_2->children.size()) return false;
  // Since the number of children match, we can iterate in parallel.
  auto it_1 = rt_1->children.begin();
  auto it_2 = rt_2->children.begin();
  while (it_1 != rt_1->children.end()) {
    // Check that the strings on the branches match.
    if (it_1->first != it_2->first) return false;
    // Recursively check for equality.
    if (!are_equal(it_1->second, it_2->second)) return false;
    ++it_1;
    ++it_2;
  }
  return true;
}

const Trie::Node* Trie::first_key(const Trie::Node* rt) {
  assert(rt);

  // If rt has no children, nullptr.
  if (rt->children.empty()) return nullptr;
  rt = rt->children.begin()->second.get();
  assert(rt);

  // Keep moving down the tree along the left side until is_end.
  while (!rt->is_end) {
    // If rt is not an end, its children should not be empty.
    assert(!rt->children.empty());
    rt = rt->children.begin()->second.get();
    assert(rt);
  }
  return rt;
}

const Trie::Node* Trie::next_node(const Trie::Node* ptr) {
  assert(ptr);

  // Go up once then keep going up until we can move right.
  auto par = ptr->parent;
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
  assert(next(child_iter) != par->children.end());
  ++child_iter;
  const auto& rn = child_iter->second;
  assert(rn);

  // Return the smallest key rooted at rn.
  if (rn->is_end) return rn.get();
  assert(!rn->children.empty());
  return first_key(rn.get());
}

string Trie::underlying_string(const Node* ptr) {
  assert(ptr);

  // As we move up, push string representations onto stack.
  stack<string> history;
  // Move up in trie until we get to root.
  auto par = ptr->parent;

  while (par) {
    // We must be able to find ptr in par->children.
    auto iter = value_find(par->children, ptr);
    assert(iter != par->children.end());

    // Push the string representation onto the stack and go up.
    history.push(iter->first);
    ptr = par;
    par = par->parent;
  }

  // If par is null, then ptr must be root. Concatenate strings in reverse.
  string str = "";
  while (!history.empty()) {
    str += history.top();
    history.pop();
  }

  return str;
}

bool Trie::check_invariant(const unique_ptr<Node>& root) {
  // Check that root is non-null.
  if (!root) return false;
  unordered_set<char> characters;

  // Check validity of children.
  for (const auto& str_ptr_pair : root->children) {
    // No null nodes in children tree.
    if (!str_ptr_pair.second) return false;
    // Ensure that its parent is root.
    if (str_ptr_pair.second->parent != root.get()) return false;
    // Make sure string is not empty.
    if (str_ptr_pair.first.empty()) return false;
    /*
    Check that string does not share a prefix with other children.
    We only really need to check first char.
    */
    if (characters.find(str_ptr_pair.first.front()) != characters.end())
      return false;
    characters.emplace(str_ptr_pair.first.front());
    // Recursively check child node.
    if (!check_invariant(str_ptr_pair.second)) return false;
  }

  // If root passes every single check, the tree is valid.
  return true;
}

Trie::Trie() : root(make_unique<Node>(false, nullptr)) {
  assert(check_invariant(root));
}

Trie::Trie(const initializer_list<string>& key_list) : Trie() {
  for (const auto& key : key_list) {
    insert(key);
  }
  assert(check_invariant(root));
}

Trie::Trie(const Trie& other) : Trie() {
  assert(other.root);
  recursive_copy(root, other.root);
  assert(check_invariant(root));
}

Trie::Trie(Trie&& other) : Trie() {
  swap(*this, other);
  assert(check_invariant(root));
}

Trie& Trie::operator=(Trie other) {
  swap(*this, other);
  assert(check_invariant(root));
  return *this;
}

bool Trie::empty(string prefix) const {
  const auto prf_rt = prefix_match(root.get(), prefix);
  // Check if prefix root is null
  if (!prf_rt) return true;
  // It's empty if prf_rt is not a word and has no children.
  assert(check_invariant(root));
  return !prf_rt->is_end && prf_rt->children.empty();
}

size_t Trie::size(string prefix) const {
  const auto prf_rt = prefix_match(root.get(), prefix);
  if (!prf_rt) return size_t(0);
  size_t counter = 0;
  key_counter(prf_rt, counter);

  assert(check_invariant(root));
  return counter;
}

Trie::iterator Trie::find(string key, bool is_prefix) const {
  // Check if we need an exact match.
  if (!is_prefix) {
    // Elegantly handles both match and no match.
    return iterator(exact_match(root.get(), key));
  }

  // In this case, we need only find a word that key is a prefix of.
  const auto prf_rt = prefix_match(root.get(), key);
  // If key is not a prefix of anything, there is no match.
  if (!prf_rt) return iterator();

  // Find the first child key rooted at prt_rt.
  assert(check_invariant(root));
  // If key is empty and prf_rt is and end node, then it is the "first key".
  return key.empty() && prf_rt->is_end ? iterator(prf_rt)
                                       : iterator(first_key(prf_rt));
}

Trie::iterator Trie::insert(string key) {
  /*
  Note: inserting key at root, is the same
  as inserting reduced key at loc.
  The problem space has been reduced.
  */
  auto loc = const_cast<Trie::Node*>(approximate_match(root.get(), key));
  assert(loc);
  /* INSERT KEY AT LOC */

  // If the key is now empty, simply set is_end to true.
  if (key.empty()) {
    loc->is_end = true;
    assert(check_invariant(root));
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
    assert(check_invariant(root));
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
      assert(check_invariant(root));
      return iterator(junction->children[post_key].get());
    } else {
      assert(check_invariant(root));
      return iterator(junction.get());
    }
  }

  // If there are no shared prefixes, then simply create a node under loc.
  {
    auto key_node = make_unique<Node>(true, loc);
    loc->children.emplace(key, std::move(key_node));
  }
  assert(check_invariant(root));
  return iterator(loc->children[key].get());
}

void Trie::erase(string key, bool is_prefix) {
  /*
  If is_prefix flag is set, wipe everything
  at and under the prefix_match.
  */
  if (is_prefix) {
    auto prf_ptr = prefix_match(root.get(), key);
    if (!prf_ptr) return;
    if (prf_ptr == root.get()) {
      clear();
    } else {
      const auto par = const_cast<Trie::Node*>(prf_ptr->parent);
      assert(par);
      par->children.erase(value_find(par->children, prf_ptr));
    }
    assert(check_invariant(root));
    return;
  }

  // Must remove exact key.
  const auto match = const_cast<Trie::Node*>(exact_match(root.get(), key));
  // If the key was not in the tree, just return.
  if (!match) return;
  // No matter what happens, setting is_end to false is correct.
  match->is_end = false;

  // If match is the root node, it won't have a parent to deal with.
  if (match == root.get()) {
    // If key was non-empty, exact_match failed.
    assert(key.empty());
    assert(check_invariant(root));
    return;
  }

  if (match->children.empty()) {
    const auto par = const_cast<Trie::Node*>(match->parent);
    auto match_iter = value_find(par->children, match);
    par->children.erase(match_iter);

    // Check for possible joining with grand parent.
    if (par->children.size() == 1 && par != root.get() && !par->is_end) {
      const auto grand_par = const_cast<Trie::Node*>(par->parent);
      assert(grand_par);
      auto par_iter = value_find(grand_par->children, par);
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
    const auto par = const_cast<Trie::Node*>(match->parent);
    assert(par);
    const auto match_iter = value_find(par->children, match);
    string joined_key = match_iter->first + only_child->first;

    only_child->second->parent = par;
    par->children.emplace(joined_key, std::move(only_child->second));
    par->children.erase(match_iter);
  }

  assert(check_invariant(root));
  // If match has multiple children, nothing can be joined.
}

void Trie::clear() {
  // Clear everything under root.
  root->children.clear();
  root->is_end = false;
  assert(!root->parent);
  assert(check_invariant(root));
}

Trie::iterator::iterator(const Node* p) : ptr(p) {}

Trie::iterator& Trie::iterator::operator++() {
  /*
  If the ptr has children, return the first child.
  Otherwise, return the next node that isn't a child.
  Elegantly handles the case when the returned value is nullptr.
  */
  ptr = ptr->children.empty() ? next_node(ptr) : first_key(ptr);
  return *this;
}

Trie::iterator Trie::iterator::operator++(int) {
  auto temp(*this);
  ++(*this);
  return temp;
}

string Trie::iterator::operator*() const { return underlying_string(ptr); }

Trie::iterator::operator bool() const { return ptr != nullptr; }

Trie::iterator Trie::begin() const {
  return root->is_end ? iterator(root.get()) : iterator(first_key(root.get()));
}

Trie::iterator Trie::end() const { return iterator(nullptr); }

Trie::iterator Trie::begin(const string& prefix) const {
  // Find the first key that matches the given prefix.
  return find(prefix, PREFIX_FLAG);
}

Trie::iterator Trie::end(string prefix) const {
  // Perform an approximate match.
  auto app_ptr = approximate_match(root.get(), prefix);
  assert(app_ptr);

  /*
  If prefix is empty, app_ptr is a prefix match and
  none of its children work. If all children of app_ptr
  are less than prefix, nothing under app_ptr works.
  */
  if (prefix.empty() || app_ptr->children.empty() ||
      app_ptr->children.rbegin()->first < prefix)
    return iterator(next_node(app_ptr));

  // Find the first child that is greater than prefix
  for (auto& str_ptr_pair : app_ptr->children) {
    // If equality, then approximate_match failed.
    assert(str_ptr_pair.first != prefix);
    if (str_ptr_pair.first.front() > prefix.front()) {
      return str_ptr_pair.second->is_end
                 ? iterator(str_ptr_pair.second.get())
                 : iterator(first_key(str_ptr_pair.second.get()));
    }
  }

  // If we've gotten down to here, something has gone wrong.
  throw runtime_error("Unexpected bug in Trie::end(std::string)");
}

Trie& Trie::operator+=(const Trie& rhs) {
  assert(this != &rhs);
  for (const string& key : rhs) {
    insert(key);
  }
  assert(check_invariant(root));
  return *this;
}

Trie operator+(Trie lhs, const Trie& rhs) { return lhs += rhs; }

Trie& Trie::operator-=(const Trie& rhs) {
  assert(this != &rhs);
  for (const string& key : rhs) {
    erase(key);
  }
  assert(check_invariant(root));
  return *this;
}

Trie operator-(Trie lhs, const Trie& rhs) { return lhs -= rhs; }

bool operator==(const Trie& lhs, const Trie& rhs) {
  return Trie::are_equal(lhs.root, rhs.root);
}

bool operator!=(const Trie& lhs, const Trie& rhs) { return !(lhs == rhs); }

bool operator<(const Trie& lhs, const Trie& rhs) {
  if (lhs.size() >= rhs.size()) return false;
  return includes(rhs.begin(), rhs.end(), lhs.begin(), lhs.end());
}

bool operator>(const Trie& lhs, const Trie& rhs) { return rhs < lhs; }

bool operator<=(const Trie& lhs, const Trie& rhs) { return !(rhs < lhs); }

bool operator>=(const Trie& lhs, const Trie& rhs) { return !(lhs < rhs); }

ostream& operator<<(std::ostream& os, const Trie& tree) {
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
