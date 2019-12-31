# Radix Tree

The API defined in `trie.h` is that of a set with prefix operations. The map is implemented as a radix tree (a compact prefix tree or trie) in modern C++. See <https://en.wikipedia.org/wiki/Radix_tree> for more information on radix trees.

## Usage

To use the radix tree, simply put `#include "trie.h"` at the beginning of your file and compile/link `trie.cpp` with the rest of your program.

Using the included `Makefile`, run `make` to compile `benchmark.cpp`, which conducts unit and performance tests.

## Documentation

By default, `prefix` and `is_prefix` parameters are empty `std::string` and `false` respectively. Best practice is to use `Trie::PREFIX_FLAG` to specify that an operation works on prefixes.

### Construction

The class comes equipped with:

- default constructor
- `initializer_list` constructor
- copy and move constructors
- range constructor

All constructors have a memory guarantee that exceptions thrown while constructing will not cause memory leaks.

### Size

The `empty` and `size` functions take a `prefix` parameter. They return, respectively:

- whether or not the tree contains keys of the given prefix
- the number of words with the given prefix

These functions do *not* modify the container.

### Searching

The `find` function returns an iterator to the key if it's contained in the tree. If `is_prefix` is set, it returns an iterator to the first key that matches the prefix.

This function does *not* modify the container.

### Insertion

To insert keys into the tree, This function has no effect if the key is already in the tree. There are 2 overloads of `insert`. It inserts a single key into the tree and returns an iterator to a Node matching the key.

### Deletion

To remove keys from the tree, use `erase`. This function has no effect if the key is not in the tree. It removes a single key from the tree. If `is_prefix` is set, erases all keys that match the prefix.

To reset the entire tree, simply call `clear`, which is idempotent on empty trees.

### Iteration

The tree supports constant forward iterators that traverse the stored keys in alphabetical order. The class comes with STL style `begin` and `end` functions that range over the entire tree. Use the `begin` and `end` overloads with the `prefix` parameter to construct ranges over keys that match prefixes.

### Operators

- Adding trees using the `+` or `+=` operators will take a set union over the contained keys.
- Subtracting trees using the `-` or `-=` operators will take a set difference over the contained keys.
- Trees can be compared using `==, !=, <, >, <=, =>` where inequality implies a subset relation.
- Printing all keys stored in the tree in alphabetical order can be done using the `<<` operator.

## Testing

Running `benchmark.cpp` executes all unit and performance tests.

### Unit Tests

The `Trie` class is validated using black box unit testing. We test the following functions.

- Default, `initializer_list`, copy, and range constructors.
- Destructor (does not leak memory).
- `empty`, `size`, `find`, `insert`, and `erase`.
- Iterator increment and dereference.
- Traversal with `begin` and `end`.
- All arithmetic and comparison operators.

The `Unit_Test` base class is a functor that handles the running and checking of test cases for the `Trie`. Each test case inherits from `Unit_Test` and implements its own constructor (initialize `answer`) and `test()` function. Each of them is added to a `vector` of `Unit_Test` types.

### Performance Tests

We also compare performance of `set<string>` and `Trie` under big data inputs. The benchmark measures the time it takes for each data structure to complete:

- Mass insertion of randomly assorted words.
- Determining the size of various prefix subsets.
- Finding the range of words with a given prefix.
- Mass deletion of all words with a given prefix.

## Invariants

1. Given a node N, children of N do not share any common non-empty prefixes. Otherwise, the common prefix would have been compressed.
2. As a corollary of (1), for any non-empty prefix P and node N, at most 1 child node of N has P as a prefix.
3. The empty string is never in a children map. Suppose N contains the empty string in its children map. This would be equivalent to N being `is_end`.
4. All leaf nodes have true `is_end`. If a leaf node N was not the end of a word, must have non-empty `children` map, which it can't have because it's a leaf.
5. If node N has false `is_end`, it must have at least 2 children node. Otherwise, it would be compressed with its only child.
6. As another corollary of (1), a children map can have at most |char| items. Therefore, we can treat searching `std::map` as constant.
7. `approximate_match`, `prefix_match`, and `exact_match` can be composed due to the recursive structure of the trie.
8. `root` is never null. The empty trie consists of a root node with false `is_end`, an empty `children` map, and `nullptr` as parent.
