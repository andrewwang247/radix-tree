# Radix Tree

The API defined in `trie.h` is that of a set with prefix operations. The map is implemented as a radix tree (a compact prefix tree or trie) in modern C++. See <https://en.wikipedia.org/wiki/Radix_tree> for more information on radix trees.

## Usage

To use the radix tree, simply put `#include "trie.h"` at the beginning of your file and compile/link with the rest of your program.

Using the included `Makefile`, run `make` to compile `benchmark.cpp`, which conducts unit and performance tests.

## Documentation

### Construction

The class comes equipped with:

- default constructor
- `initializer_list` constructor
- copy and move constructors
- range constructor

### Size

The `empty` and `size` functions take a `prefix` parameter that is empty by default. They return, respectively:

- whether or not the tree contains keys of the given prefix
- the number of keys with the given prefix

Since an empty string is a prefix for *all* strings, default behavior yields the expected container behavior for emptiness and size checking. These functions do *not* modify the container.

### Searching

The `find` function returns an iterator to the key if it's contained in the tree. The `find_prefix` function returns an iterator to the first key that matches the prefix. These functions do *not* modify the container.

### Insertion

The `insert` function adds a single key into the tree and returns an iterator to a Node matching the key. The function is idempotent.

### Deletion

To remove keys from the tree, use `erase`. The `erase_prefix` erases all keys that match the prefix. To reset the entire tree, simply call `clear`. Both `erase` variants and `clear` are idempotent.

### Iteration

The tree supports constant forward iterators that traverse the stored keys in alphabetical order. The class comes with STL style `begin` and `end` functions that range over the entire tree. Use the `begin` and `end` overloads with `prefix` parameter to construct ranges over keys that match prefixes. Make sure to check that `begin(std::string prefix)` is non-null before using as a range. This can be efficiently achieved with `empty(std::string prefix)`.

### Operators

- Adding trees using the `+` or `+=` operators will take a set union over the contained keys.
- Subtracting trees using the `-` or `-=` operators will take a set difference over the contained keys.
- Trees can be compared using `==, !=, <, >, <=, =>` where inequality implies a subset relation.
- Printing all keys stored in the tree in alphabetical order can be done using the `<<` operator.

## Testing

Running `benchmark.cpp` executes all unit and performance tests. In addition, the code has been checked for memory leaks using valgrind.

### Unit Tests

The `Trie` class is validated with 8 black box unit tests. We test the following functions.

- Default, `initializer_list`, copy, and range constructors.
- Destructor (recursively deletes allocations).
- `empty`, `size`, `find`, `insert`, and `erase`.
- Iterator increment and dereference.
- Traversal with `begin` and `end`.
- All arithmetic and comparison operators.

### Performance Tests

The performance of `std::set<std::string>` and `Trie` are compared under big data inputs. The benchmark measures the time it takes for each data structure to complete:

- Mass insertion of randomly assorted keys.
- Determining the size of various prefix subsets.
- Finding the range of keys with a given prefix.
- Mass deletion of all keys with a given prefix.
- Iterating over the entire container.

## Invariants

This section discusses implementation details. It's not needed to write client code.

1. Given a node *N*, children of *N* do not share any common non-empty prefixes. Otherwise, the common prefix would have been compressed.
2. As a corollary of (1), for any non-empty prefix *p* and node *N*, at most 1 child node of *N* has *p* as a prefix.
3. The empty string is never in a children map. Suppose *N* contains the empty string in its children map. This would be equivalent to *N* being `is_end`.
4. All leaf nodes have true `is_end`. If a leaf node *N* was not the end of a key, must have non-empty `children` map, which it can't have because it's a leaf.
5. If node *N* has false `is_end`, it must have at least 2 children node. Otherwise, it would be compressed with its only child.
6. As another corollary of (1), a children map can have at most `|char|` items. Therefore, we can treat searching `std::map` as constant.
7. `approximate_match`, `prefix_match`, and `exact_match` can be composed due to the recursive structure of the trie.
8. `root` is never null. The empty trie consists of a root node with false `is_end`, an empty `children` map, and `nullptr` as parent.
