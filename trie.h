#pragma once
#include <string>
#include <set>
#include <initializer_list>
#include <vector>
#include <ostream>

// * Possible future update: trie multiset with optional parameter in node specifying frequency.

/**
 * Defines a singular node in the Trie data structure.
 */
struct Node {
	// Flag for when this node represents the end of a word.
	bool is_end;
	// Set of pointers to the children node.
	std::set<char, Node*> children;
};

/**
 * A prefix tree with char as alphabet. By convention, the empty
 * string is always considered as being contained in the trie.
 */
class Trie {
public:
	/* --- CONSTRUCTORS, DESTRUCTORS, UTILITY --- */

	// Default constructor initializes empty trie.
	Trie() noexcept;

	// Initializes tree with a singular entry.
	Trie( const std::string& word );

	// Initializer list constructor inserts strings in list into trie.
	Trie( const std::initializer_list<std::string>& list );

	/**
	 * Templated iterator constructor inserts all items between constructors.
	 * REQUIRES: Input_Iterator is of type or inherited from std::input_iterator.
	 */
	template <typename InputIterator>
	Trie( InputIterator start, InputIterator finish );

	// Copy constructor.
	Trie( const Trie& other );

	// Move constructor.
	Trie( Trie&& other );

	// Non-throwing destructor.
	~Trie() noexcept;

	// Assignment operator.
	Trie& operator=( Trie other );

	/* --- CORE ALGORITHMS --- */

	/**
	 * Returns whether or not the trie is empty.
	 */
	bool empty() const noexcept;

	/**
	 * Searches for word in trie and returns whether search was successful.
	 * If is_prefix is true, it treats the word as a prefix and returns whether
	 * or not the trie contains the given prefix. By default, is_prefix is
	 * false, so the word is searched as a complete string.
	 */
	bool contains( const std::string& word, bool is_prefix = false ) const noexcept;

	/**
	 * Inserts word into trie. Idempotent if word in trie.
	 */
	void insert( const std::string& word );

	/**
	 * Erases word from trie. Idempotent if word not in trie.
	 * If is_prefix is true, deletes everything with word as prefix.
	 * By default, is_prefix is false, so it erases a singular word.
	 */
	void erase( const std::string& word, bool is_prefix = false ) noexcept;

	/*
	Templated iterator versions of contains, insert, and erase.
	*/

	// Returns if at least one call of contains( *InputIterator, is_prefix ) is true.
	template <typename InputIterator>
	bool contains_any_of( InputIterator start, InputIterator finish, bool is_prefix = false ) const noexcept;

	// Returns if every call of contains( *InputIterator, is_prefix ) is true.
	template <typename InputIterator>
	bool contains_all_of( InputIterator start, InputIterator finish, bool is_prefix = false ) const noexcept;

	// Returns if every call of contains( *InputIterator, is_prefix ) is false.
	template <typename InputIterator>
	bool contains_none_of( InputIterator start, InputIterator finish, bool is_prefix = false ) const noexcept;

	// Inserts each element defined in the range [start, finish).
	template <typename InputIterator>
	void insert( InputIterator start, InputIterator finish );

	// Erases each element defined in the range [start, finish).
	template <typename InputIterator>
	void erase( InputIterator start, InputIterator finish, bool is_prefix = false ) noexcept;

	/**
	 * Erases all words from trie. Idempotent on empty tries.
	 * Equivalent to erase( "", true ).
	 */
	void reset() const noexcept;

	/**
	 * Returns the number of words stored in the trie with given prefix.
	 * Default prefix is empty, which means the full trie size is returned.
	 */
	size_t size( const std::string& prefix = "" ) const noexcept;

	/* --- ENTRY OUTPUT --- */

	/**
	 * Returns a sorted vector of all entries with given prefix.
	 * Default prefix is empty, which means the entire trie is included.
	 */
	std::vector<std::string> entry_list( const std::string& prefix = "" ) const;

	/**
	 * Writes a sorted list of all entries with given prefix into the output iterator.
	 * Returns an iterator the end of the destination range.
	 */
	template <typename OutputIterator>
	OutputIterator copy( OutputIterator start, const std::string& prefix = "" ) const;

	/* --- ASYMMETRIC BINARY OPERATIONS --- */

	/*
	Define addition and subtraction of tries as the union and
	set difference of the entries contained in the trie.
	*/
	
	/* Arithmetic operators */

	Trie& operator+=( const Trie& rhs );
	Trie& operator-=( const Trie& rhs ) noexcept;

private:
	// Pointer to the root node.
	Node* root;
};

/* --- SYMMETRIC BINARY OPERATIONS --- */

/*
Two tries are equal if they contain their entry lists are equivalents.
Trie A is strictly less than Trie B if Trie A's entry list is a proper
subset of Trie B's entry list.
*/

/* Comparison operators */

inline bool operator==( const Trie& lhs, const Trie& rhs ) noexcept;
inline bool operator!=( const Trie& lhs, const Trie& rhs ) noexcept;
inline bool operator<( const Trie& lhs, const Trie& rhs ) noexcept;
inline bool operator>( const Trie& lhs, const Trie& rhs ) noexcept;
inline bool operator<=( const Trie& lhs, const Trie& rhs ) noexcept;
inline bool operator>=( const Trie& lhs, const Trie& rhs ) noexcept;

/* Arithmetic operators */

inline Trie operator+( Trie lhs, const Trie& rhs );
inline Trie operator-( Trie lhs, const Trie& rhs );

/**
 * Outputs each entry in obj to os. Each entry is given its own line.
 */
std::ostream& operator<<( std::ostream& os, const Trie& obj );
