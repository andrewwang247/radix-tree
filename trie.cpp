/*
Copyright 2020. Siwei Wang.

Implementation for Trie.
*/
#include "trie.h"

#include <algorithm>
#include <map>
#include <memory>
#include <stack>
#include <unordered_set>
#include <utility>
using std::initializer_list;
using std::make_shared;
using std::map;
using std::move;
using std::ostream;
using std::runtime_error;
using std::shared_ptr;
using std::stack;
using std::string;
using std::swap;
using std::unordered_set;

Trie::Node::Node(bool is_end_in, std::shared_ptr<Node> parent_in)
    : is_end(is_end_in), parent(parent_in), children() {}

void Trie::recursive_copy(std::shared_ptr<Node> rt,
                          const std::shared_ptr<Node> other) {
  assert(rt && other);
  // Make rt's is_end the same as other's is_end.
  rt->is_end = other->is_end;
  // Recursively copy children.
  for (const auto& str_ptr_pair : other->children) {
    const auto child = make_shared<Node>(false, rt);
    rt->children.emplace(str_ptr_pair.first, std::move(child));
    recursive_copy(child, str_ptr_pair.second);
  }
}

bool Trie::is_prefix(const string& prf, const string& word) {
  // The empty string is a prefix for every string.
  if (prf.empty()) return true;
  // Assuming non-emptiness of prf, it cannot be longer than word.
  if (prf.length() > word.length()) return false;
  // std::algorithm function that returns iterators to first mismatch.
  auto res = mismatch(prf.begin(), prf.end(), word.begin());

  // If we reached the end of prf, it's a prefix.
  return res.first == prf.end();
}

std::shared_ptr<Trie::Node> Trie::approximate_match(
    const std::shared_ptr<Node> rt, string& key) {
  assert(rt);
  // If the key is empty, return the root node.
  if (key.empty()) return rt;

  for (const auto& str_ptr_pair : rt->children) {
    assert(str_ptr_pair.second);
    // If one of the children is a prefix of key, recurse.
    if (is_prefix(str_ptr_pair.first, key)) {
      // Remove the child string off the front of key.
      return approximate_match(str_ptr_pair.second,
                               key.erase(0, str_ptr_pair.first.length()));
    }
  }

  // If none of the children form a prefix for key, simply return the root.
  return rt;
}

std::shared_ptr<Trie::Node> Trie::prefix_match(const std::shared_ptr<Node> rt,
                                               string& prf) {
  // First compute the approximate root.
  auto app_ptr = approximate_match(rt, prf);
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
      return str_ptr_pair.second;
    }
  }

  // No way to make prf a prefix. Return null.
  return nullptr;
}

void Trie::key_counter(const std::shared_ptr<Node> rt, size_t& acc) {
  assert(rt);
  // If root contains a word, increment the counter.
  if (rt->is_end) ++acc;
  // Recursively check for words in children
  for (const auto& str_ptr_pair : rt->children) {
    assert(str_ptr_pair.second);
    key_counter(str_ptr_pair.second, acc);
  }
}

std::shared_ptr<Trie::Node> Trie::exact_match(const std::shared_ptr<Node> rt,
                                              string word) {
  // First compute the approximate root.
  auto app_ptr = approximate_match(rt, word);
  assert(app_ptr);
  /*
  If the given word is empty, it's a perfect match.
  Otherwise, there is no match.
  */
  return word.empty() ? app_ptr : nullptr;
}

bool Trie::are_equal(const std::shared_ptr<Node> rt_1,
                     const std::shared_ptr<Node> rt_2) {
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

std::shared_ptr<Trie::Node> Trie::first_key(std::shared_ptr<Node> rt) {
  assert(rt);

  // If rt has no children, nullptr.
  if (rt->children.empty()) return nullptr;
  rt = rt->children.begin()->second;
  assert(rt);

  // Keep moving down the tree along the left side until is_end.
  while (!rt->is_end) {
    // If rt is not an end, its children should not be empty.
    assert(!rt->children.empty());
    rt = rt->children.begin()->second;
    assert(rt);
  }
  return rt;
}

std::shared_ptr<Trie::Node> Trie::next_node(std::shared_ptr<Node> ptr) {
  assert(ptr);

  // Go up once then keep going up until we can move right.
  auto par = ptr->parent.lock();
  // Note that par->children cannot be empty since its a parent.
  assert(!par->children.empty());
  while (par && par->children.rbegin()->second == ptr) {
    // Move up.
    ptr = par;
    par = par->parent.lock();
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
  auto rn = child_iter->second;
  assert(rn);

  // Return the smallest key rooted at rn.
  if (rn->is_end) return rn;
  assert(!rn->children.empty());
  return first_key(rn);
}

string Trie::underlying_string(std::shared_ptr<Node> ptr) {
  assert(ptr);

  // As we move up, push string representations onto stack.
  stack<string> history;
  // Move up in trie until we get to root.
  auto par = ptr->parent.lock();

  while (par) {
    // We must be able to find ptr in par->children.
    auto iter = value_find(par->children, ptr);
    assert(iter != par->children.end());

    // Push the string representation onto the stack and go up.
    history.push(iter->first);
    ptr = par;
    par = par->parent.lock();
  }

  // If par is null, then ptr must be root. Concatenate strings in reverse.
  string str = "";
  while (!history.empty()) {
    str += history.top();
    history.pop();
  }

  return str;
}

bool Trie::check_invariant(const shared_ptr<Node> root) {
  // Check that root is non-null.
  if (!root) return false;
  unordered_set<char> characters;

  // Check validity of children.
  for (const auto& str_ptr_pair : root->children) {
    // No null nodes in children tree.
    if (!str_ptr_pair.second) return false;
    // Ensure that its parent is root.
    if (str_ptr_pair.second->parent.lock() != root) return false;
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

Trie::Trie() : root(make_shared<Node>(false, nullptr)) {
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
  const auto prf_rt = prefix_match(root, prefix);
  // Check if prefix root is null
  if (!prf_rt) return true;
  // It's empty if prf_rt is not a word and has no children.
  assert(check_invariant(root));
  return !prf_rt->is_end && prf_rt->children.empty();
}

size_t Trie::size(string prefix) const {
  const auto prf_rt = prefix_match(root, prefix);
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
    return iterator(exact_match(root, key));
  }

  // In this case, we need only find a word that key is a prefix of.
  const auto prf_rt = prefix_match(root, key);
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
  auto loc = approximate_match(root, key);
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
    auto child = make_shared<Node>(true, loc);
    loc->children.emplace(key, child);
    assert(check_invariant(root));
    return iterator(child);
  }

  // Check children of loc for shared prefixes.
  for (const auto& str_ptr_pair : loc->children) {
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

    // This node will be moved under junction.
    auto old_child = str_ptr_pair.second;
    // Create a child for the common part. junction's parent is set.
    auto junction = make_shared<Node>(post_key.empty(), loc);
    // Add junction to loc under common.
    loc->children.emplace(common, junction);
    // loc child is added to junction's children map.
    junction->children.emplace(post_child, old_child);
    // The original child's parent pointer is set to junction.
    old_child->parent = junction;
    // Remove child_str from loc child map.
    loc->children.erase(child_str);

    if (!post_key.empty()) {
      // Add an additional node for the split.
      auto key_node = make_shared<Node>(true, junction);
      junction->children.emplace(post_key, key_node);

      assert(check_invariant(root));
      return iterator(key_node);
    } else {
      assert(check_invariant(root));
      return iterator(junction);
    }
  }

  // If there are no shared prefixes, then simply create a node under loc.
  auto key_node = make_shared<Node>(true, loc);
  loc->children.emplace(key, key_node);
  assert(check_invariant(root));
  return iterator(key_node);
}

void Trie::erase(string key, bool is_prefix) {
  /*
  If is_prefix flag is set, wipe everything
  at and under the prefix_match.
  */
  if (is_prefix) {
    auto prf_ptr = prefix_match(root, key);
    if (!prf_ptr) return;
    if (prf_ptr == root) {
      clear();
    } else {
      auto par = prf_ptr->parent.lock();
      assert(par);
      par->children.erase(value_find(par->children, prf_ptr));
    }
    assert(check_invariant(root));
    return;
  }

  // Must remove exact key.
  auto match = exact_match(root, key);
  // If the key was not in the tree, just return.
  if (!match) return;
  // No matter what happens, setting is_end to false is correct.
  match->is_end = false;

  // If match is the root node, it won't have a parent to deal with.
  if (match == root) {
    // If key was non-empty, exact_match failed.
    assert(key.empty());
    assert(check_invariant(root));
    return;
  }

  if (match->children.empty()) {
    auto par = match->parent.lock();
    auto match_iter = value_find(par->children, match);
    par->children.erase(match_iter);

    // Check for possible joining with grand parent.
    if (par->children.size() == 1 && par != root && !par->is_end) {
      auto grand_par = par->parent.lock();
      assert(grand_par);
      auto par_iter = value_find(grand_par->children, par);
      assert(par_iter != grand_par->children.end());

      // Join keys on par_iter and the only child of par.
      string mod_key = par_iter->first + par->children.begin()->first;
      auto child = par->children.begin()->second;

      grand_par->children.emplace(mod_key, child);
      child->parent = grand_par;
      grand_par->children.erase(par_iter);
    }
  } else if (match->children.size() == 1) {
    // Extract child and parent string to form joined key.
    auto only_child = match->children.begin();
    auto par = match->parent.lock();
    assert(par);
    auto match_iter = value_find(par->children, match);
    string joined_key = match_iter->first + only_child->first;

    only_child->second->parent = par;
    par->children.emplace(joined_key, only_child->second);
    par->children.erase(match_iter);
  }

  assert(check_invariant(root));
  // If match has multiple children, nothing can be joined.
}

void Trie::clear() {
  // Clear everything under root.
  root->children.clear();
  root->is_end = false;
  assert(!root->parent.lock());
  assert(check_invariant(root));
}

Trie::iterator::iterator(const std::shared_ptr<Node> p) : ptr(p) {}

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

string Trie::iterator::operator*() { return underlying_string(ptr); }

Trie::iterator::operator bool() const { return ptr != nullptr; }

Trie::iterator Trie::begin() const {
  return root->is_end ? iterator(root) : iterator(first_key(root));
}

Trie::iterator Trie::end() const { return iterator(nullptr); }

Trie::iterator Trie::begin(string prefix) const {
  // Find the first key that matches the given prefix.
  return find(prefix, PREFIX_FLAG);
}

Trie::iterator Trie::end(string prefix) const {
  // Perform an approximate match.
  auto app_ptr = approximate_match(root, prefix);
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
                 ? iterator(str_ptr_pair.second)
                 : iterator(first_key(str_ptr_pair.second));
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
