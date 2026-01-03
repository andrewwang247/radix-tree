# Radix Tree

The API defined in `trie.h` is that of a set with prefix operations. The map is implemented as a radix tree (a compact prefix tree or trie) in C++17. See <https://en.wikipedia.org/wiki/Radix_tree> for more information on radix trees.

## Usage

To use the radix tree, simply put `#include "trie.h"` at the beginning of your file and compile/link with the rest of your program.

Use the included `Makefile` to build `benchmark`. Run with the:

- `debug` option to run unit tests to assert correctness
- `release` option to run performance tests against `std::set`

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

The `insert` function adds a single key into the tree and returns an iterator to a node matching the key. The function is idempotent.

### Deletion

To remove keys from the tree, use `erase`. The `erase_prefix` erases all keys that match the prefix. To reset the entire tree, simply call `clear`. Both `erase` variants and `clear` are idempotent.

### Iteration

The tree supports constant bidirectional iterators that traverse the stored keys in alphabetical order. The class comes with STL style `begin` and `end` functions that range over the entire tree. Use the `begin` and `end` overloads with `prefix` parameter to construct ranges over keys that match prefixes. Make sure to check that `begin(std::string prefix)` is non-null before using as a range. This can be efficiently achieved with `empty(std::string prefix)`.

### Operators

- Adding trees using the `+` or `+=` operators will take a set union over the contained keys.
- Subtracting trees using the `-` or `-=` operators will take a set difference over the contained keys.
- Trees can be compared using `==, !=, <, >, <=, =>` where inequality implies a subset relation.
- Printing all keys stored in the tree in alphabetical order can be done using the `<<` operator.

### Representation

The tree and its iterators can output an internal representation of the node structure in JSON format using the `to_json` function. For iterators, the representation is of the sub-tree rooted at itself. This function takes an optional boolean flag that enables showing where end nodes are. For the full data representation, this should be turned on. That said, disabling end markers makes the resulting JSON visually cleaner and more human readable. For example, the words:

- `compute`
- `computer`
- `contain`
- `contaminate`
- `corn`
- `corner`
- `mahjong`
- `mahogany`
- `mat`
- `material`
- `maternal`
- `math`
- `matrix`

are represented internally with the following prefix structure (omitting end markers).

```json
{
  "co": {
    "mpute": {
      "r": {}
    },
    "nta": {
      "in": {},
      "minate": {}
    },
    "rn": {
      "er": {}
    }
  },
  "ma": {
    "h": {
      "jong": {},
      "ogany": {}
    },
    "t": {
      "er": {
        "ial": {},
        "nal": {}
      },
      "h": {},
      "rix": {}
    }
  }
}
```

## Testing

Running `benchmark.cpp` executes either unit or performance tests, depending on the build options. In addition, the code has been checked for memory leaks using valgrind.

### Unit Tests

In the debug build, all assertions are enabled and we frequently validate the structural invariants. The `trie` class is tested for the following functionality:

- Default, `initializer_list`, and iterator range constructors.
- Copy and move semantics for constructors and assignment.
- `empty` and `size` both with and without a prefix parameter.
- `find` and `find_prefix` to search for keys and ranges.
- `insert`, `empty`, and `empty_prefix` to modify the `trie`.
- Bidirectional iterator functionality as a return type from other functions.
- Traversal from `begin` to `end` both with and without a prefix parameter.
- All arithmetic and comparison operators for 1 or more `trie`.
- `to_json` representation both with and without end markers.

### Performance Tests

In the release build, assertions are disabled and we benchmark the performance of `std::set<std::string>` and `trie` under large inputs. The benchmark measures the time it takes for each data structure to complete:

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
