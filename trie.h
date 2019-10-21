#pragma once
#include <string>
#include <map>
#include <initializer_list>
#include <iostream>
#include <exception>
#include <cassert>

/**
 * An compact prefix tree with keys as std::basic_string.
 * The empty string is always contained in the trie.
 */
class Trie {
private:

	/**
	 * Defines a singular node in the Trie data structure. If it corresponds
	 * to a full key, the is_end flag is set to true.
	 */
	struct Node {
		// Flag for when the given Node ends a full word. False by default.
		bool is_end = false;
		// Set of pointers to the children node.
		std::map<std::string, Node*> children;
		// Pointer to the parent node, null by default.
		Node* parent = nullptr;
	};

	/**
	 * Private member variable.
	 * @param root is nullptr if the trie is empty.
	 *     Otherwise, it's points to the root node.
	 */
	Node* root;

	/* --- HELPER FUNCTIONS --- */

	
	// TODO: helper function that returns the pointer at the head of a given prefix parameter.
	// TODO: helper function that, given a pointer to a node, constructs the corresponding string.

	/**
	 * Helper function.
	 * Recursively copies other into rt.
	 * REQUIRES: rt and other are not nullptr.
	 * THROWS: std::bad_alloc if @new fails.
	 */
	void recursive_copy( Node* rt, const Node* other );

	/**
	 * Helper function.
	 * Recursively deletes all nodes that are children
	 * of rt as well as rt itself.
	 */
	void recursive_delete( Node* rt ) noexcept;

	/*
	Radix tree invariants.
	1. Given a node N, children of N do not share any common non-empty prefixes.
		Otherwise, the common prefix would have been compressed.
	2. As a corollary of (1), for any non-empty prefix P and node N, at most 1 child
		node of N has P as a prefix.
	3. The empty string is never in a children map. Suppose N contains the empty string
		in its children map. This would be equivalent to N being is_end.
	4. All leaf nodes have true is_end. If a leaf node N was not the end of a word,
		it must have non-empty children map, which it can't have because it's a leaf.
	5. If node N has false is_end, it must have at least 2 children node.
		Otherwise, it would be compressed with its only child.
	6. As another corollary of (1), a children map can have at most |char| items.
		Therefore, we can treat searching std::map as constant.
	*/

	/**
	 * Helper function.
	 * RETURNS: whether or not prf is a prefix of word.
	 * @param prf: the string to match with the beginning of word.
	 * @param word: the full string for which we are testing existence of a prefix.
	 */
	bool is_prefix( const std::string& prf, const std::string& word ) const noexcept;

	/**
	 * Helper function.
	 * Depth traversing search for the deepest node N such that a prefix of key
	 * matches the string representation at N.
	 * RETURNS: the node N described above.
	 * GUARANTEES: since the root node is equivalent to the empty string,
	 *    this function will never return a nullptr UNLESS rt is empty.
	 * MODIFIES: key such that the string representation at N is removed.
	 * @param rt: the node at which to start searching.
	 * @param key: the key on which to make an approximate match.
	 */
	Node* approximate_match( const Node* rt, std::string& key ) const noexcept;

	/**
	 * Helper function.
	 * Depth traversing search for the node that serves as a root for prf.
	 * RETURNS: the deepest node N such that N and all of N's children have prf as prefix.
	 *     if rt is nullptr, returns a nullptr.
	 *     if prf is not a prefix, returns a nullptr.
	 * MODIFIES: prf so that the string at prefix_match is removed from prf.
	 *     Note that if prf is not a prefix, the modified prf reflects as far as it got.
	 * @param rt: the node at which to start searching.
	 * @param prf: the prefix which the return node should be a root of.
	 */
	Node* prefix_match( const Node* rt, std::string& prf ) const noexcept;

	/**
	 * Helper function.
	 * Depth traversing search for the node that matches word.
	 * RETURNS: the node that forms an exact match with the given word.
	 *     if no match is found, returns a nullptr.
	 * @param rt: the root node from which to search.
	 * @param word: the string we are trying to match.
	 */
	Node* exact_match( const Node* rt, std::string& word ) const noexcept;

	/**
	 * Helper function.
	 * Counts the number of keys stored at or as children of rt added to acc.
	 * Equivalent to counting the number of true is_end's accessible from rt.
	 * @param rt: the root node at which to start counting.
	 * @param acc: the value at which to start counting.
	 */
	void key_counter( const Node* rt, size_t& acc ) const noexcept;

public:

	/**
	 * Default constructor initializes empty trie.
	 */
	Trie() noexcept;

	/* --- INITIALIZER LIST CONSTRUCTORS --- */

	/**
	 * Initializer list constructor inserts strings in key_list into trie.
	 * Duplicates are ignored.
	 * GUARANTEES: No memory leaks if exception is thrown.
	 * @param key_list: The items to initialize the trie with.
	 */
	Trie( const std::initializer_list<std::string>& key_list );

	/* --- ITERATOR CONSTRUCTORS --- */

	/**
	 * Initializes with all items in the range [key_start, key_finish).
	 * Duplicates keys are ignored.
	 * REQUIRES: InputIterator over strings and has base class std::input_iterator.
	 * GUARANTEES: No memory leaks if exception is thrown.
	 * @param key_start: begin iterator to the range.
	 * @param key_finish: end iterator to the range. 
	 */
	template <typename InputIterator>
	Trie( InputIterator key_start, InputIterator key_finish ) : Trie() {
		try {
			insert( key_start, key_finish );
		}
		catch( std::bad_alloc& e ) {
			std::cerr << e.what() << std::endl;
			recursive_delete( root );
		}
	}

	/* --- DYNAMIC MEMORY: RULE OF 5 */

	/**
	 * Copy constructor.
	 * GUARANTEES: No memory leaks if exception is thrown.
	 * @param other: the trie to copy into this.
	 */
	Trie( const Trie& other );

	/**
	 * Move constructor.
	 * @param other: the trie to move into this.
	 */
	Trie( Trie&& other ) noexcept;

	/**
	 * Destructor.
	 */
	~Trie() noexcept;

	/**
	 * Assignment operator.
	 * @param other: the trie to assign to this.
	 */
	Trie& operator=( Trie other ) noexcept;

	/* --- CONTAINER SIZE --- */

	/**
	 * RETURNS: whether or not the trie is empty starting at given prefix.
	 * Prefix defaults to empty string, corresponding to entire trie.
	 * @param prefix: the prefix on which to check for emptiness.
	 */
	bool empty( const std::string& prefix = "" ) const noexcept;

	/**
	 * RETURNS: the number of words stored in the trie with given prefix.
	 * Default prefix is empty, which means the full trie size is returned.
	 * @param prefix: the prefix on which to check for size.
	 */
	size_t size( const std::string& prefix = "" ) const noexcept;

	/* --- ITERATION --- */

	/**
	 * Supports bidirectional const iteration over the trie.
	 */
	class iterator {
	private:
		/**
		 * Reference to the underlying trie.
		 */
		const Trie& tree;
		/**
		 * The current Node being pointed at.
		 */
		const Node* ptr;
	public:
		/**
		 * Constructor, Node ptr is null by default.
		 * @param t: the trie reference to assign to tree.
		 * @param p: the Node that the iterator is currently pointing at.
		 */
		iterator( const Trie& t, const Node* p = nullptr ) noexcept;
		/**
		 * Prefix increment.
		 */
		iterator& operator++();
		/**
		 * Postfix increment.
		 */
		iterator operator++(int);
		/**
		 * Prefix decrement.
		 */
		iterator& operator--();
		/**
		 * Postfix decrement.
		 */
		iterator operator--(int);
		/**
		 * Dereference operator.
		 */
		std::string operator*();
	};

	/**
	 * RETURNS: iterator the beginning of the trie.
	 */
	iterator begin() const noexcept;

	/**
	 * RETURNS: iterator to one past the end of the trie.
	 */
	iterator end() const noexcept;

	/*
	Prefix traversal by iterator. Returns begin and end iterators to the range of
	items which has prefix given by the parameter. Note that they constitute an
	alphabetically ordered range like regular traversal by iterator.
	If none of the keys have the given prefix, returns a null iterator.
	begin("") and end("") have the same behavior as begin() and end()
	since every key has empty string as prefix.
	*/

	/**
	 * RETURNS: iterator to the start of the range with given prefix.
	 * @param prefix: the prefix for which the function returns a begin iterator to.
	 */
	iterator begin( const std::string& prefix ) const noexcept;

	/**
	 * RETURNS: iterator to one past the end of the range with given prefix.
	 * @param prefix: the prefix for which the function returns an end iterator to.
	 */
	iterator end( const std::string& prefix ) const noexcept;

	/* --- SEARCHING --- */

	/**
	 * Searches for key in trie.
	 * RETURNS: an iterator to it if it exists.
	 *     Otherwise, returns a null iterator.
	 * If is_prefix is true, returns an iterator to the first
	 * key that matches the prefix.
	 * @param key: The key used to search the trie.
	 * @param is_prefix: Flags whether or not to treat the key as a prefix.
	 */
	iterator find( std::string key, bool is_prefix = false ) const noexcept;

	/* --- INSERTION --- */

	/**
	 * Inserts key (or key pointed to by iterator) into trie.
	 * GUARANTEES: Idempotent if key in trie.
	 * RETURNS: an iterator to the key (whether inserted or not).
	 * @param key: the key to insert into the trie.
	 */
	iterator insert( const std::string& key );
	/**
	 * Same as regular insertion, but performs an insertion operation
	 * on every item in the range [start, finish).
	 * REQUIRES: InputIterator over strings and has base class std::input_iterator.
	 * @param start: begin iterator to the range.
	 * @param finish: end iterator to the range. 
	 */
	template <typename InputIterator>
	void insert( InputIterator start, InputIterator finish ) {
		while ( start != finish ) {
			insert( *start );
			++start;
		}
	}
	/**
	 * Same as regular insertion, but performs insert on initializer list.
	 * @param list: the items to insert into the trie.
	 */
	void insert( const std::initializer_list<std::string>& list );

	/* --- DELETION --- */

	/**
	 * Erases key (or key pointed to by iterator) from trie.
	 * GUARANTEES: Idempotent if key ( or prefix ) is not in trie.
	 * RETURNS: iterator after the item erased.
	 * @param key: the key to erase from the trie.
	 */
	iterator erase( const std::string& key ) noexcept;
	/**
	 * Same as regular deletion, but performs an erase operation
	 * on every item in the range [start, finish).
	 * REQUIRES: InputIterator over strings and has base class std::input_iterator.
	 * @param start: begin iterator to the range.
	 * @param finish: end iterator to the range. 
	 */
	template <typename InputIterator>
	void erase( InputIterator start, InputIterator finish ) {
		while ( start != finish ) {
			erase( *start );
			++start;
		}
	}
	/**
	 * Same as regular deletion, but performs erase on initializer list.
	 * @param list: the items to erase from the trie.
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

	/**
	 * Inserts all of rhs's keys into this.
	 * @param rhs: the trie to union with this.
	 */
	Trie& operator+=( const Trie& rhs );
	/**
	 * Removes all of rhs's keys from this.
	 * @param rhs: the trie to set subtract from this.
	 */
	Trie& operator-=( const Trie& rhs ) noexcept;

};

/* --- SYMMETRIC BINARY OPERATIONS --- */

/*
COMPARISON OF TRIES.
We say that A == B if A and B have equivalent keys.
Define A < B as a proper subset relation.
*/

inline bool operator==( const Trie& lhs, const Trie& rhs ) noexcept;
inline bool operator!=( const Trie& lhs, const Trie& rhs ) noexcept;
inline bool operator<( const Trie& lhs, const Trie& rhs ) noexcept;
inline bool operator>( const Trie& lhs, const Trie& rhs ) noexcept;
inline bool operator<=( const Trie& lhs, const Trie& rhs ) noexcept;
inline bool operator>=( const Trie& lhs, const Trie& rhs ) noexcept;

// Arithmetic operators, uses += and -=.

inline Trie operator+( Trie lhs, const Trie& rhs );
inline Trie operator-( Trie lhs, const Trie& rhs );

/**
 * Outputs each entry in tree to os. Each entry is given its own line.
 */
std::ostream& operator<<( std::ostream& os, const Trie& tree );

// Comparison between iterators performs element-wise comparison.

inline bool operator==( const Trie::iterator& lhs, const Trie::iterator& rhs ) noexcept;
inline bool operator!=( const Trie::iterator& lhs, const Trie::iterator& rhs ) noexcept;