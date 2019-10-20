# Radix Tree

The API defined in `trie.h` is that of a templated set with prefix operations. The map is implemented as a radix tree (a compact prefix tree or trie) in modern C++17. See <https://en.wikipedia.org/wiki/Radix_tree> for more information on radix trees.

## Usage

To use the radix tree, simply put `#include "trie.h"` at the beginning of your file and compile/link `trie.cpp` with the rest of your program.

Using the included `Makefile`, run `make` to compile and execute all included test cases in the `tests` directory.

## Documentation

By default, `prefix` and `is_prefix` parameters are empty `std::string` and `false` respectively.

### Construction

The class comes equipped with:

- default constructor
- initializer list constructor
- iterator constructor
- copy and move constructors

All constructors have a memory guarantee that exceptions thrown while constructing will not cause memory leaks.

The default constructor initializes an empty trie. The initializer list constructors take lists of keys. The iterator constructors can iterate over keys in the given range.

Duplicates are ignored.

### Size

The `empty` and `size` functions take a `prefix` parameter. They return, respectively:

- whether or not the tree contains keys of the given prefix
- the number of words with the given prefix

These functions do *not* modify the container.

### Searching

The `contains` function returns whether or not the given key is contained in the tree. If `is_prefix` is set, it returns whether the tree contains any words with the given key as prefix.

This function does *not* modify the container.

### Insertion

To insert keys into the tree, use the `insert` function. If the key is already contained in the tree, the function call has no effect. If it's not yet in the tree, it will be inserted.

### Deletion

To remove a singular key or all keys matching a given prefix, use `erase` with the appropriate `is_prefix` parameter. This function has no effect if a key match is not found.

To reset the entire tree, simply call `clear`, which is idempotent on empty trees.

### Iteration

The tree supports bidirectional iterators (of regular, const, reverse, and const reverse varieties) that traverse the stored keys in alphabetical or reverse alphabetical order.

### Operators

- Adding trees using the `+` or `+=` operators will take a set union over the contained keys.
- Subtracting trees using the `-` or `-=` operators will take a set difference instead.
- Trees can be compared using `==, !=, <, >, <=, =>` where inequality implies a subset relation.
- Printing all keys stored in the tree in alphabetical order can be done using the `<<` operator.
