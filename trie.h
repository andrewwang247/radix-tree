#pragma once
#include <string>
#include <map>
#include <initializer_list>
#include <iostream>
#include <exception>
#include <cassert>

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
 * Defines helper functions that are used to implement member functions.
 * TODO: helper function that returns the pointer at the head of a given prefix parameter.
 * TODO: helper function that, given a pointer to a node, constructs the corresponding string.
 */
namespace Helper {

	/**
	 * Helper function
	 * Recursively copies other into root.
	 * REQUIRES: root and other are not nullptr.
	 * THROWS: std::bad_alloc if @new fails.
	 */
	void recursive_copy( Node* root, const Node* other ) {
		assert( !root && !other );
		// Make root's is_end the same as other's is_end.
		root->is_end = other->is_end;
		// Recursively copy children.
		for ( const auto& x : other->children ) {
			Node* child = new Node;
			root->children[x.first] = child;
			recursive_copy( child, x.second );
		}
	}

	/**
	 * Helper function
	 * Recursively deletes all nodes that are children
	 * of root as well as root itself.
	 */
	void recursive_delete( Node* root ) noexcept {
		if ( !root ) return;
		// Recursively delete all children.
		for ( auto str_ptr_pair : root->children ) {
			recursive_delete( str_ptr_pair.second );
		}
		delete root;
	}

}

/**
 * An compact prefix tree with keys as std::basic_string.
 * The empty string is always contained in the trie.
 */
class Trie {
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
			Helper::recursive_delete( root );
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
	iterator find( const std::string& key, bool is_prefix = false ) const noexcept;

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