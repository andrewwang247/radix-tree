#pragma once
#include <string>
#include <unordered_set>
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
	std::unordered_set<char, Node*> children;
};

/**
 * A prefix tree with char as alphabet. The empty string is
 * is always considered as being contained in the trie.
 * In general, function behavior when is_prefix is true is a
 * supserset of its behavior when is_prefix is false.
 */
class Trie {
public:
	/* --- CONSTRUCTORS, DESTRUCTORS, UTILITY --- */

	// Default constructor initializes empty trie.
	Trie() noexcept;

	// Initializes tree with a singular entry.
	explicit Trie( const std::string& word );

	// Initializer list constructor inserts strings in list into trie.
	Trie( const std::initializer_list<std::string>& list );

	/**
	 * Initializes with all items between input iterators.
	 * REQUIRES: InputIterator over strings and has base class std::input_iterator.
	 */
	template <typename InputIterator>
	Trie( InputIterator start, InputIterator finish );

	/* Managing dynamic memory: the rule of 5. */

	Trie( const Trie& other );
	Trie( Trie&& other );
	~Trie() noexcept;
	Trie& operator=( Trie other );

	/* --- CORE ALGORITHMS --- */

	/**
	 * Returns whether or not the trie is empty starting at given prefix.
	 * Prefix defaults to empty string, corresponding to entire trie.
	 */
	bool empty( const std::string& prefix = "" ) const noexcept;

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

	/**
	 * Erases all words from trie. Idempotent on empty tries.
	 * Equivalent result to erase( "", true ).
	 */
	void reset() const noexcept;

	/**
	 * Returns the number of words stored in the trie with given prefix.
	 * Default prefix is empty, which means the full trie size is returned.
	 */
	size_t size( const std::string& prefix = "" ) const noexcept;

	/* --- ITERATOR VERSIONS --- */

	// NOTE: close parallels between templated contains functions and STL algorithm's any_of, all_of, and none_of.

	/**
	 * Returns if at least one call of contains( *InputIterator, is_prefix ) is true.
	 * REQUIRES: InputIterator over strings and has base class std::input_iterator.
	 */
	template <typename InputIterator>
	bool contains_any_of( InputIterator start, InputIterator finish, bool is_prefix = false ) const noexcept;

	/**
	 * Returns if every call of contains( *InputIterator, is_prefix ) is true.
	 * REQUIRES: InputIterator over strings and has base class std::input_iterator.
	 */
	template <typename InputIterator>
	bool contains_all_of( InputIterator start, InputIterator finish, bool is_prefix = false ) const noexcept;

	/**
	 * Returns if every call of contains( *InputIterator, is_prefix ) is false.
	 * REQUIRES: InputIterator over strings and has base class std::input_iterator.
	 */
	template <typename InputIterator>
	bool contains_none_of( InputIterator start, InputIterator finish, bool is_prefix = false ) const noexcept;

	/**
	 * Inserts each element defined in the range [start, finish).
	 * REQUIRES: InputIterator over strings and has base class std::input_iterator.
	 */
	template <typename InputIterator>
	void insert( InputIterator start, InputIterator finish );

	/**
	 * Erases each element defined in the range [start, finish).
	 * REQUIRES: InputIterator over strings and has base class std::input_iterator.
	 */
	template <typename InputIterator>
	void erase( InputIterator start, InputIterator finish, bool is_prefix = false ) noexcept;

	/* --- ENTRY OUTPUT --- */

	/**
	 * Returns a sorted vector of all entries with given prefix.
	 * Default prefix is empty, which means the entire trie is included.
	 * If trie is empty or no prefix matches, returns empty vector.
	 */
	std::vector<std::string> entry_list( const std::string& prefix = "" ) const;

	/**
	 * Writes a sorted list of all entries with given prefix into the output iterator.
	 * Returns an iterator to the end of the destination range.
	 * REQUIRES: OutputIterator over strings and has base class std::output_iterator.
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

	// TODO: helper function that returns the pointer at the head of a given prefix parameter.
};

/* --- SYMMETRIC BINARY OPERATIONS --- */

/*
Two tries are equal if the entry lists they contain are equivalents.
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
