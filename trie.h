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
 * 
 * Radix tree invariants.
 * 1. Given a node N, children of N do not share any common non-empty prefixes.
 *     Otherwise, the common prefix would have been compressed.
 * 2. As a corollary of (1), for any non-empty prefix P and node N, at most 1
 *     child node of N has P as a prefix.
 * 3. The empty string is never in a children map. Suppose N contains the empty
 *     string in its children map. This would be equivalent to N being is_end.
 * 4. All leaf nodes have true is_end. If a leaf node N was not the end of a word,
 *     must have non-empty children map, which it can't have because it's a leaf.
 * 5. If node N has false is_end, it must have at least 2 children node.
 *     Otherwise, it would be compressed with its only child.
 * 6. As another corollary of (1), a children map can have at most |char| items.
 *     Therefore, we can treat searching std::map as constant.
 * 7. approximate_match, prefix_match, and exact_match can be composed due
 *     to the recursive structure of the trie.
 * 8. root is never null. The empty trie consists of a root node with false is_end,
 *     an empty children set, and nullptr as parent.
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
	 * Private member variable pointing to the root node.
	 */
	Node* root;

	/* --- HELPER FUNCTIONS --- */

	/**
	 * Helper function.
	 * Recursively copies other into rt.
	 * REQUIRES: rt and other are not null but empty.
	 * THROWS: std::bad_alloc if @new fails.
	 */
	static void recursive_copy( Node* const rt, const Node* const other );

	/**
	 * Helper function.
	 * Recursively deletes all nodes that are children
	 * of rt as well as rt itself.
	 */
	static void recursive_delete( Node* rt ) noexcept;

	/**
	 * Helper function.
	 * RETURNS: whether or not prf is a prefix of word.
	 * @param prf: The string to match with the beginning of word.
	 * @param word: The full string for which we are testing existence of a prefix.
	 */
	static bool is_prefix( const std::string& prf, const std::string& word ) noexcept;

	/**
	 * Helper function.
	 * Depth traversing search for the deepest node N such that a prefix of key
	 * matches the string representation at N.
	 * REQUIRES: rt is not null.
	 * RETURNS: The node N described above.
	 * GUARANTEES: Since the root node is equivalent to the empty string,
	 *    this function will never return a nullptr.
	 * MODIFIES: Key such that the string representation at N is removed.
	 * @param rt: The node at which to start searching.
	 * @param key: The key on which to make an approximate match.
	 */
	static Node* approximate_match( const Node* const rt, std::string& key ) noexcept;

	/**
	 * Helper function.
	 * Depth traversing search for the node that serves as a root for prf.
	 * REQUIRES: rt is not null.
	 * RETURNS: The deepest node N such that N and all of N's children have prf as prefix.
	 *     If prf is not a prefix, returns a nullptr.
	 * MODIFIES: prf so that the string at prefix_match is removed from prf.
	 *     Note that if prf is not a prefix, the modified prf reflects as far as it got.
	 * @param rt: The node at which to start searching.
	 * @param prf: The prefix which the return node should be a root of.
	 */
	static Node* prefix_match( const Node* const rt, std::string& prf ) noexcept;

	/**
	 * Helper function.
	 * Depth traversing search for the node that matches word.
	 * REQUIRES: rt is not null.
	 * RETURNS: The first node that exactly matches the given word.
	 *     If no match is found, returns a nullptr.
	 * @param rt: The root node from which to search.
	 * @param word: The string we are trying to match.
	 */
	static Node* exact_match( const Node* const rt, std::string word ) noexcept;

	/**
	 * Helper function.
	 * Counts the number of keys stored at or as children of rt added to acc.
	 * Equivalent to counting the number of true is_end's accessible from rt.
	 * REQUIRES: rt is not null.
	 * @param rt: The root node at which to start counting.
	 * @param acc: The value at which to start counting.
	 */
	static void key_counter( const Node* const rt, size_t& acc ) noexcept;

	/**
	 * RETURNS: Whether or not the tries rooted at rt_1 and rt_2 are equivalent.
	 * REQUIRES: rt_1 and rt_2 are not null.
	 * @param rt_1: The root of the first trie.
	 * @param rt_2: The root of the second trie.
	 */
	static bool are_equal( const Node* const rt_1, const Node* const rt_2 ) noexcept;

	/**
	 * Searches for the the given value in a map.
	 * RETURNS: An iterator to the position which matches val.
	 *     This is the end iterator if val is not in the map.
	 * @param m: The map on which to search.
	 * @param val: The Node we are searching for in the map.
	 */
	static std::map<std::string, Node*>::iterator value_find( const std::map<std::string, Node*>& m, const Node* const val ) noexcept;

	/**
	 * RETURNS: The first key that is a child of rt.
	 * REQUIRES: rt is not null.
	 * @param rt: The root node at which to start.
	 */
	static Node* first_key( const Node* rt ) noexcept;

	/**
	 * RETURNS: The first key AFTER ptr that is not a child of ptr.
	 *    If there isn't such a key, returns nullptr.
	 * REQUIRES: ptr is not null.
	 * GUARANTEES: The returned Node is_end.
	 * @param ptr: Starting node position.
	 */
	static Node* next_over( const Node* ptr ) noexcept;

	/**
	 * RETURNS: The string representation at ptr.
	 * REQUIRES: Parent pointers along ptr are not cyclic (infinite loop)
	 * @param ptr: The node for which we are trying to construct a string.
	 */
	static std::string underlying_string( const Node* const ptr ) noexcept;

	/**
	 * RETURNS: Whether or not the tree at root is valid (satisfies invariants).
	 * @param root: The root of the tree to check.
	 */
	static bool check_invariant( const Node* const root ) noexcept;

public:

	/**
	 * Used to mark a parameter as passing in a prefix and not a full key.
	 */
	static constexpr bool PREFIX_FLAG = true;

	/**
	 * Default constructor initializes empty trie.
	 */
	Trie();

	/* --- INITIALIZER LIST CONSTRUCTORS --- */

	/**
	 * Initializer list constructor inserts strings in key_list into trie.
	 * Duplicates are ignored.
	 * GUARANTEES: No memory leaks if exception is thrown.
	 * @param key_list: The items to initialize the trie with.
	 */
	Trie( const std::initializer_list<std::string>& key_list );

	/* --- DYNAMIC MEMORY: RULE OF 5 */

	/**
	 * Copy constructor.
	 * GUARANTEES: No memory leaks if exception is thrown.
	 * @param other: The trie to copy into this.
	 */
	Trie( const Trie& other );

	/**
	 * Move constructor.
	 * @param other: The trie to move into this.
	 */
	Trie( Trie&& other ) noexcept;

	/**
	 * Destructor.
	 */
	~Trie() noexcept;

	/**
	 * Assignment operator.
	 * @param other: The trie to assign to this.
	 */
	Trie& operator=( Trie other ) noexcept;

	/* --- CONTAINER SIZE --- */

	/**
	 * RETURNS: Whether or not the trie is empty starting at given prefix.
	 * Prefix defaults to empty string, corresponding to entire trie.
	 * @param prefix: The prefix on which to check for emptiness.
	 */
	bool empty( const std::string& prefix = "" ) const noexcept;

	/**
	 * RETURNS: The number of words stored in the trie with given prefix.
	 * Default prefix is empty, which means the full trie size is returned.
	 * @param prefix: The prefix on which to check for size.
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
		 * @param t: The trie reference to assign to tree.
		 * @param p: The Node that the iterator is currently pointing at.
		 */
		iterator( const Trie& t, const Node* const p = nullptr ) noexcept;
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
		/**
		 * Implicit conversion to bool.
		 * RETURNS: Whether or not the underlying pointer is null.
		 */
		operator bool() const;
	};

	/**
	 * RETURNS: Iterator to the beginning of the trie.
	 */
	iterator begin() const noexcept;

	/**
	 * RETURNS: Iterator to one past the end of the trie.
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
	 * RETURNS: Iterator to the start of the range with given prefix.
	 * @param prefix: The prefix for which the function returns a begin iterator to.
	 */
	iterator begin( const std::string& prefix ) const noexcept;

	/**
	 * RETURNS: Iterator to one past the end of the range with given prefix.
	 * @param prefix: The prefix for which the function returns an end iterator to.
	 */
	iterator end( std::string prefix ) const noexcept;

	/* --- SEARCHING --- */

	/**
	 * Searches for key in trie.
	 * RETURNS: An iterator to it if it exists.
	 *     Otherwise, returns a null iterator.
	 * If is_prefix is true, returns an iterator to the first
	 * key that matches the prefix.
	 * @param key: The key used to search the trie.
	 * @param is_prefix: Flags whether or not to treat the key as a prefix.
	 */
	iterator find( std::string key, bool is_prefix = !PREFIX_FLAG ) const noexcept;

	/* --- INSERTION --- */

	/**
	 * Inserts key (or key pointed to by iterator) into trie.
	 * GUARANTEES: Idempotent if key in trie.
	 * RETURNS: An iterator to the key (whether inserted or not).
	 * @param key: The key to insert into the trie.
	 */
	iterator insert( std::string key );

	/* --- DELETION --- */

	/**
	 * Erases key from trie. If is_prefix, erases all keys that have
	 * the key as prefix from the trie.
	 * GUARANTEES: Idempotent if key ( or prefix ) is not in trie.
	 * @param key: The key to erase from the trie.
	 * @param is_prefix: Flag for treating key as a prefix.
	 */
	void erase( const std::string& key, bool is_prefix = !PREFIX_FLAG ) noexcept;

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
	 * REQUIRES: This and rhs are not the same trie.
	 * @param rhs: The trie to union with this.
	 */
	Trie& operator+=( const Trie& rhs );
	/**
	 * Removes all of rhs's keys from this.
	 * REQUIRES: This and rhs are not the same trie.
	 * @param rhs: The trie to set subtract from this.
	 */
	Trie& operator-=( const Trie& rhs ) noexcept;

	// Private access for == operators to allow efficient deep equality check. See COMPARISON OF TRIES.
	friend inline bool operator==( const Trie& lhs, const Trie& rhs ) noexcept;
	friend inline bool operator<( const Trie& lhs, const Trie& rhs ) noexcept;
};

/* --- SYMMETRIC BINARY OPERATIONS --- */

/*
COMPARISON OF TRIES.
We say that A == B if A and B have equivalent keys.
Define A < B as a proper subset relation.
*/

inline bool operator!=( const Trie& lhs, const Trie& rhs ) noexcept;
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