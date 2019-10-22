# Radix Tree

The API defined in `trie.h` is that of a set with prefix operations. The map is implemented as a radix tree (a compact prefix tree or trie) in modern C++. See <https://en.wikipedia.org/wiki/Radix_tree> for more information on radix trees.

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

The `find` function returns an iterator to the key if it's contained in the tree. If `is_prefix` is set, it returns an iterator to the first key that matches the prefix.

This function does *not* modify the container.

### Insertion

To insert keys into the tree, This function has no effect if the key is already in the tree. There are 2 overloads of `insert`.

- Inserts a single key. Returns iterator to inserted key.
- Inserts an initializer list of strings.

### Deletion

To remove keys from the tree, use `erase`. This function has no effect if the key is not in the tree. There are 2 overloads of `erase`.

- Erase a single key. If `is_prefix` is set, erases all keys that match the prefix.
- Erase an initializer list of strings.

To reset the entire tree, simply call `clear`, which is idempotent on empty trees.

### Iteration

The tree supports constant bidirectional iterators that traverse the stored keys in alphabetical order. The class comes with STL style `begin` and `end` functions that range over the entire tree. Use the `begin` and `end` overloads with the `prefix` parameter to construct ranges over keys that match prefixes.

### Operators

- Adding trees using the `+` or `+=` operators will take a set union over the contained keys.
- Subtracting trees using the `-` or `-=` operators will take a set difference over the contained keys.
- Trees can be compared using `==, !=, <, >, <=, =>` where inequality implies a subset relation.
- Printing all keys stored in the tree in alphabetical order can be done using the `<<` operator.
