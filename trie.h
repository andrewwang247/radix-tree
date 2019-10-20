#pragma once
#include <string>
#include <map>
#include <initializer_list>
#include <ostream>
#include <iterator>

/**
 * Defines a singular node in the Trie data structure. If it corresponds
 * to a full key, it may store a datum of templated type.
 */
struct Node {
	// Flag for when the given Node ends a full word. False by default.
	bool is_end = false;
	// Set of pointers to the children node.
	std::map<std::string, Node*> children;
};

/**
 * An compact prefix tree with keys as std::basic_string.
 * The empty string is always contained in the trie.
 * In general, function behavior when is_prefix is true is a
 * supserset of its behavior when is_prefix is false.
 */
class Trie {
public:

	/**
	 * Default constructor initializes empty trie.
	 * GUARANTEES: No memory leaks if exception is thrown.
	 */
	Trie() noexcept;

	/* --- INITIALIZER LIST CONSTRUCTORS --- */

	/**
	 * Initializer list constructor inserts strings in key_list into trie.
	 * Duplicates are ignored.
	 * GUARANTEES: No memory leaks if exception is thrown.
	 */
	Trie( const std::initializer_list<std::string>& key_list );

	/* --- ITERATOR CONSTRUCTORS --- */

	/**
	 * Initializes with all items between input iterators.
	 * Duplicates keys are ignored.
	 * REQUIRES: InputIterator over strings and has base class std::input_iterator.
	 * GUARANTEES: No memory leaks if exception is thrown.
	 */
	template <typename InputIterator>
	Trie( InputIterator key_start, InputIterator key_finish );

	/* --- DYNAMIC MEMORY: RULE OF 5 */

	// Copy constructor
	Trie( const Trie& other );

	// Move constructor
	Trie( Trie&& other );

	// Destructor
	~Trie() noexcept;

	// Assignment operator
	Trie& operator=( Trie other );

	/* --- CONTAINER SIZE --- */

	/**
	 * Returns whether or not the trie is empty starting at given prefix.
	 * Prefix defaults to empty string, corresponding to entire trie.
	 */
	bool empty( const std::string& prefix = "" ) const noexcept;

	/**
	 * Returns the number of words stored in the trie with given prefix.
	 * Default prefix is empty, which means the full trie size is returned.
	 */
	size_t size( const std::string& prefix = "" ) const noexcept;

	/* --- SEARCHING --- */

	/**
	 * Searches for key in trie and returns whether search was successful.
	 * If is_prefix is true, it treats the key as a prefix and returns whether
	 * or not the trie contains the given prefix. By default, is_prefix is
	 * false, so the key is searched as a complete string.
	 */
	bool contains( const std::string& key, bool is_prefix = false ) const noexcept;

	/* --- INSERTION --- */

	/**
	 * Inserts key into trie.
	 * GUARANTEES: Idempotent if key in trie.
	 * Returns whether or not a key was inserted.
	 */
	bool insert( const std::string& key );

	/* --- DELETION --- */

	/**
	 * Erases key from trie.
	 * If is_prefix is true, deletes everything with key as prefix.
	 * If is_prefix is false, only erases a singular key.
	 * GUARANTEES: Idempotent if key ( or prefix ) is not in trie.
	 * Returns the number of keys erased.
	 */
	size_t erase( const std::string& key, bool is_prefix = false ) noexcept;

	/**
	 * Erases all keys from trie.
	 * Equivalent result to erase( "", true ).
	 * GUARANTEES: Idempotent on empty tries.
	 */
	void clear() noexcept;

	/* --- ITERATION --- */

	class iterator {
	private:
		Trie& tree;
		Node* p;
	public:
		// constructor
		iterator(Trie& t, Node* p = nullptr);
		// increment operators
		iterator& operator++();
		iterator operator++(int);
		// decrement operators
		iterator& operator--();
		iterator operator--(int);
		// dereference operator
		std::string& operator*();
	};

	class const_iterator {
	private:
		const Trie& tree;
		const Node* p;
	public:
		// constructor
		const_iterator(const Trie& t, const Node* p = nullptr);
		// increment operators
		const_iterator& operator++();
		const_iterator operator++(int);
		// decrement operators
		const_iterator& operator--();
		const_iterator operator--(int);
		// dereference operator
		const std::string& operator*() const;
	};

	// Return begin and end iterators of the regular and const varieties.

	iterator begin() noexcept;
	iterator end() noexcept;
	const_iterator begin() const noexcept;
	const_iterator end() const noexcept;

	/* --- ASYMMETRIC BINARY OPERATIONS --- */

	/*
	A + B inserts all of B's keys into A.
	A - B erases all of B's keys from A.
	*/

	Trie& operator+=( const Trie& rhs );
	Trie& operator-=( const Trie& rhs ) noexcept;

private:

	// Pointer to the root node.
	Node* root;
};

/* --- SYMMETRIC BINARY OPERATIONS --- */

/*
COMPARISON OF TRIES.
We say that A == B if A and B have equivalent keys.
Define A < B as a proper subset relation.
*/

// Comparison operators

inline bool operator==( const Trie& lhs, const Trie& rhs ) noexcept;
inline bool operator!=( const Trie& lhs, const Trie& rhs ) noexcept;
inline bool operator<( const Trie& lhs, const Trie& rhs ) noexcept;
inline bool operator>( const Trie& lhs, const Trie& rhs ) noexcept;
inline bool operator<=( const Trie& lhs, const Trie& rhs ) noexcept;
inline bool operator>=( const Trie& lhs, const Trie& rhs ) noexcept;

// Arithmetic operators

inline Trie operator+( Trie lhs, const Trie& rhs );
inline Trie operator-( Trie lhs, const Trie& rhs );

/**
 * Outputs each entry in tree to os. Each entry is given its own line.
 */
std::ostream& operator<<( std::ostream& os, const Trie& tree );

// Comparison between iterators.

inline bool operator==( const Trie::iterator& lhs, const Trie::iterator& rhs ) noexcept;
inline bool operator!=( const Trie::iterator& lhs, const Trie::iterator& rhs ) noexcept;

inline bool operator==( const Trie::const_iterator& lhs, const Trie::const_iterator& rhs ) noexcept;
inline bool operator!=( const Trie::const_iterator& lhs, const Trie::const_iterator& rhs ) noexcept;