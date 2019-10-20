#pragma once
#include <string>
#include <map>
#include <initializer_list>
#include <ostream>

/**
 * Defines a singular node in the Trie data structure. If it corresponds
 * to a full key, the is_end flag is set to true.
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

	/* --- ITERATION --- */

	// Supports bidirectional const iteration over the trie.
	class iterator {
	private:
		const Trie& tree;
		const Node* ptr;
	public:
		// constructor
		iterator( const Trie& t, const Node* p = nullptr );
		// increment operators
		iterator& operator++();
		iterator operator++(int);
		// decrement operators
		iterator& operator--();
		iterator operator--(int);
		// dereference operator
		std::string operator*();
	};

	// Return begin and end iterators.

	iterator begin() const noexcept;
	iterator end() const noexcept;

	/*
	Prefix traversal by iterator. Returns begin and end iterators to the range of
	items which has prefix given by the parameter. Note that they constitute an
	alphabetically ordered range like regular traversal by iterator.
	If none of the keys have the given prefix, returns a null iterator.
	begin("") and end("") have the same behavior as begin() and end()
	since every key has empty string as prefix.
	*/

	iterator begin( const std::string& prefix ) const noexcept;
	iterator end( const std::string& prefix ) const noexcept;

	/* --- SEARCHING --- */

	/**
	 * Searches for key in trie and returns an iterator to
	 * it if it exists. Otherwise, returns a null iterator.
	 * If is_prefix is true, returns an iterator to the first
	 * key that matches the prefix.
	 */
	iterator find( const std::string& key, bool is_prefix = false ) const noexcept;

	/* --- INSERTION --- */

	/**
	 * Inserts key (or key pointed to by iterator) into trie.
	 * GUARANTEES: Idempotent if key in trie.
	 * Returns an iterator to the key (whether inserted or not).
	 */
	iterator insert( const std::string& key );
	/**
	 * Same as regular insertion, but performs an insertion operation
	 * on every item in the range [start, finish).
	 */
	void insert( iterator start, iterator finish );
	/**
	 * Same as regular insertion, but performs on initializer list.
	 */
	void insert( const std::initializer_list<std::string>& list );

	/* --- DELETION --- */

	/**
	 * Erases key (or key pointed to by iterator) from trie.
	 * GUARANTEES: Idempotent if key ( or prefix ) is not in trie.
	 * Returns iterator after the item erased.
	 */
	iterator erase( const std::string& key ) noexcept;
	/**
	 * Same as regular erase, but over the range [start, finish).
	 */
	void erase( iterator start, iterator finish );
	/**
	 * Same as regular insertion, but performed on initializer list.
	 */
	void erase( const std::initializer_list<std::string>& list );

	/**
	 * Erases all keys from trie.
	 * GUARANTEES: Idempotent on empty tries.
	 */
	void clear() noexcept;

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