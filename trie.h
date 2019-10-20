#pragma once
#include <string>
#include <map>
#include <initializer_list>
#include <vector>
#include <ostream>
#include <optional>
#include <utility>

/**
 * Defines a singular node in the Trie data structure. If it corresponds
 * to a full key, it may store a datum of templated type.
 */
template <typename ValueType>
struct Node {
	// Contains data if the given Node is a full word.
	std::optional<ValueType> datum;
	// Set of pointers to the children node.
	std::map<std::string, Node*> children;
};

/**
 * An associative prefix tree with keys as std::basic_string.
 * The empty string is always contained in the trie.
 * In general, function behavior when is_prefix is true is a
 * supserset of its behavior when is_prefix is false.
 */
template <typename ValueType>
class Trie {
public:

	// Uses word as prefix.
	constexpr static bool PREFIX_FLAG = true;

	/**
	 * Default constructor initializes empty trie.
	 * GUARANTEES: No memory leaks if exception is thrown.
	 */
	Trie() noexcept;

	/* --- INITIALIZER LIST CONSTRUCTORS --- */

	/**
	 * Initializer list constructor inserts strings in key_list into trie.
	 * Duplicates are ignored.
	 * Values of each word is default initialized.
	 * REQUIRES: ValueType can be default constructed.
	 * GUARANTEES: No memory leaks if exception is thrown.
	 */
	Trie( const std::initializer_list<std::string>& key_list );

	/**
	 * Initializer list constructor inserts strings in key_list with values in value_list.
	 * Duplicates keys with multiple values will have the final value inserted.
	 * REQUIRES: key_list and value_list have the same size.
	 * GUARANTEES: No memory leaks if exception is thrown.
	 */
	Trie( const std::initializer_list<std::string& key_list, const std::initializer_list<ValueType>& value_list );

	/**
	 * Initializer list constructor inserts key value pairs into trie.
	 * Duplicates keys with multiple values will have the final value inserted.
	 * GUARANTEES: No memory leaks if exception is thrown.
	 */
	Trie( const std::initializer_list<std::pair<std::string, ValueType>>& pair_list );

	/* --- ITERATOR CONSTRUCTORS --- */

	/**
	 * If includes_values = false, initializes with items between iterators. Duplicates ignored.
	 * If includes_values = true, treats start, finish as iterators over pairs.
	 * REQUIRES: If includes_values = false,
	 *     InputIterator over strings and has base class std::input_iterator.
	 *     ValueType can be default constructed.
	 * REQUIRES: If includes_values = true,
	 *     InputIterator over pair< string, ValueType > and has base class std::input_iterator.
	 * GUARANTEES: No memory leaks if exception is thrown.
	 */
	template <typename InputIterator>
	Trie( InputIterator start, InputIterator finish, bool includes_values = false );

	/**
	 * Initializes with all items between input iterators and starting at value iterator.
	 * Duplicates keys with multiple values will take the value of the final value inserted.
	 * REQUIRES: KeyInputIterator over strings and has base class std::input_iterator.
	 * REQUIRES: ValueInputIterator over ValueType and has base class std::input_iterator.
	 * REQUIRES: value_start can be incremented the appropriate number of times.
	 * GUARANTEES: No memory leaks if exception is thrown.
	 */
	template <typename KeyInputIterator, typename ValueInputIterator>
	Trie( KeyInputIterator key_start, KeyInputIterator key_finish, ValueInputIterator value_start );

	/* --- DYNAMIC MEMORY: RULE OF 5 */

	Trie( const Trie& other );
	Trie( Trie&& other );
	~Trie() noexcept;
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
	 * Searches for word in trie and returns whether search was successful.
	 * If is_prefix is true, it treats the word as a prefix and returns whether
	 * or not the trie contains the given prefix. By default, is_prefix is
	 * false, so the word is searched as a complete string.
	 */
	bool contains( const std::string& word, bool is_prefix = false ) const noexcept;

	/* --- ELEMENT ACCESS --- */

	/**
	 * Returns reference to value stored at word.
	 * THROWS: std::out_of_range if word is not in Trie.
	 * GUARANTEES: No change to trie if throws.
	 */
	ValueType& at( const std::string& word );
	const ValueType& at( const std::string& word ) const;

	/**
	 * If word is a key, returns reference to value stored at word.
	 * Otherwise, inserts word into the trie and return reference
	 * to default constructed value.
	 * REQUIRES: ValueType can be default constructed.
	 */
	ValueType& operator[]( const std::string& word );

	/* --- IDEMPOTENT INSERTION --- */

	/**
	 * Inserts word into trie.
	 * REQUIRES: ValueType can be default constructed.
	 * GUARANTEES: Idempotent if word in trie.
	 */
	void insert( const std::string& word );

	/**
	 * Inserts pair into Trie.
	 * GUARANTEES: Idempotent if word in trie.
	 */
	void insert( const std::string& word, const ValueType& value );
	void insert( const std::pair< std::string, ValueType >& pair  );

	/* --- ASSIGNMENT INSERTION --- */

	/**
	 * Inserts pair into Trie. If key is already present,
	 * assigns value to the old key.
	 */
	void insert_or_assign( const std::string& word, const ValueType& value );
	void insert_or_assign( const std::pair< std::string, ValueType >& pair );

	/* --- ERASURE --- */

	/**
	 * Erases word from trie.
	 * If is_prefix is true, deletes everything with word as prefix.
	 * If is_prefix is false, only erases a singular word.
	 * GUARANTEES: Idempotent if word ( or prefix ) is not in trie.
	 */
	void erase( const std::string& word, bool is_prefix = false ) noexcept;

	/**
	 * Erases all words from trie.
	 * Equivalent result to erase( "", true ).
	 * GUARANTEES: Idempotent on empty tries.
	 */
	void clear() const noexcept;

	/* --- ENTRY LISTS --- */

	/**
	 * Writes a sorted list of all entries with given prefix into the output iterator.
	 * Returns an iterator to the end of the destination range.
	 * REQUIRES: OutputIterator over strings or pairs and has base class std::output_iterator.
	 */
	template <typename OutputIterator>
	OutputIterator copy_keys( OutputIterator start, const std::string& prefix = "" ) const;
	template <typename OutputIterator>
	OutputIterator copy_pairs( OutputIterator start, const std::string& prefix = "" ) const;

	/* --- ASYMMETRIC BINARY OPERATIONS --- */

	/*
	RULES: Binary operations.
	Let A and B be two distinct tries over a common ValueType.
	A + B = A with the pairs in B that not contained in A.
	That is, duplicates keys in B are ignored.
	A - B = A with all keys in B erased.
	A * B = A with all pairs in B. That is, duplicate keys
	in B overwrite those in A.
	Note that A * B = A - B + B.
	WARNINGS:
		None of the binary operations are commutative.
		There is no division operator.
	*/

	Trie& operator+=( const Trie& rhs );
	Trie& operator-=( const Trie& rhs ) noexcept;
	Trie& operator*=( const Trie& rhs );

private:

	// Pointer to the root node.
	Node<ValueType>* root;

	// TODO: helper function that returns the pointer at the head of a given prefix parameter.
};

/* --- SYMMETRIC BINARY OPERATIONS --- */

/*
COMPARISON OF TRIES.
We say that A == B if A and B have equivalent key value pairs.
Define A < B if the function given by A is a proper restriction of that given by B.
That is, A's key value pairs form a proper subset of B's key value pairs.

One can form many identities from this. If A < B, then:
	A + B = A * B = B
	A - B = empty.
*/

// Comparison operators

template <typename ValueType>
inline bool operator==( const Trie<ValueType>& lhs, const Trie<ValueType>& rhs ) noexcept;
template <typename ValueType>
inline bool operator!=( const Trie<ValueType>& lhs, const Trie<ValueType>& rhs ) noexcept;
template <typename ValueType>
inline bool operator<( const Trie<ValueType>& lhs, const Trie<ValueType>& rhs ) noexcept;
template <typename ValueType>
inline bool operator>( const Trie<ValueType>& lhs, const Trie<ValueType>& rhs ) noexcept;
template <typename ValueType>
inline bool operator<=( const Trie<ValueType>& lhs, const Trie<ValueType>& rhs ) noexcept;
template <typename ValueType>
inline bool operator>=( const Trie<ValueType>& lhs, const Trie<ValueType>& rhs ) noexcept;

// Arithmetic operators

template <typename ValueType>
inline Trie<ValueType> operator+( Trie<ValueType> lhs, const Trie<ValueType>& rhs );
template <typename ValueType>
inline Trie<ValueType> operator-( Trie<ValueType> lhs, const Trie<ValueType>& rhs );
template <typename ValueType>
inline Trie<ValueType> operator*( Trie<ValueType> lhs, const Trie<ValueType>& rhs );

/**
 * Outputs each entry in obj to os. Each entry is given its own line.
 * Format is key : value
 * REQUIRES: ValueType has operator<< defined.
 */
template <typename ValueType>
std::ostream& operator<<( std::ostream& os, const Trie<ValueType>& obj );
