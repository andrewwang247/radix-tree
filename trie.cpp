#include "trie.h"
#include <algorithm>
using namespace std;

void Trie::recursive_copy( Node* rt, const Node* other ) {
	assert( rt && other );
	// Make rt's is_end the same as other's is_end.
	rt->is_end = other->is_end;
	// Recursively copy children.
	for ( const auto& str_ptr_pair : other->children ) {
		Node* child = new Node;
		rt->children[str_ptr_pair.first] = child;
		recursive_copy( child, str_ptr_pair.second );
	}
}

void Trie::recursive_delete( Node* rt ) {
	if ( !rt ) return;
	// Recursively delete all children.
	for ( auto& str_ptr_pair : rt->children ) {
		recursive_delete( str_ptr_pair.second );
	}
	delete rt;
}

bool Trie::is_prefix( const string& prf, const string& word ) {
	// The empty string is a prefix for every string.
	if ( prf.empty() ) return true;
	// Assuming non-emptiness of prf, it cannot be longer than word.
	if ( prf.length() > word.length() ) return false;
	// std::algorithm function that returns iterators to first mismatch.
	auto res = mismatch( prf.cbegin(), prf.cend(), word.cbegin() );

	// If we reached the end of prf, it's a prefix.
	return res.first == prf.cend();
}

Trie::Node* Trie::approximate_match( const Node* rt, string& key ) {
	assert( rt );
	// If the key is empty, return the root node.
	if ( key.empty() ) return const_cast<Node*>(rt);

	for ( const auto& str_ptr_pair : rt->children ) {
		assert( str_ptr_pair.second );
		// If one of the children is a prefix of key, recurse.
		if ( is_prefix( str_ptr_pair.first, key ) ) {
			// Remove the child string off the front of key.
			return approximate_match( str_ptr_pair.second, key.erase( 0, str_ptr_pair.first.length() ) );
		}
	}

	// If none of the children form a prefix for key, simply return the root.
	return const_cast<Node*>( rt );
}

Trie::Node* Trie::prefix_match( const Node* rt, string& prf ) {
	// First compute the approximate root.
	Node* app_ptr = approximate_match( rt, prf );
	assert( app_ptr );
	// If the given prf is empty, it's a perfect match.
	if ( prf.empty() ) return app_ptr;

	// If any of the returned node's children have prf as prefix, return that child.
	for ( const auto& str_ptr_pair : app_ptr->children ) {
		assert( str_ptr_pair.second );
		if ( is_prefix( prf, str_ptr_pair.first ) ) {
			prf.clear();
			return str_ptr_pair.second;
		}
	}

	// No way to make prf a prefix. Return null.
	return nullptr;
}

void Trie::key_counter( const Node* rt, size_t& acc ) noexcept {
	assert( rt );
	// If root contains a word, increment the counter.
	if ( rt->is_end ) ++acc;
	// Recursively check for words in children
	for ( const auto& str_ptr_pair : rt->children ) {
		assert( str_ptr_pair.second );
		key_counter( str_ptr_pair.second, acc );
	}
}

Trie::Node* Trie::exact_match( const Node* rt, string& word ) {
	// First compute the approximate root.
	Node* app_ptr = approximate_match( rt, word );
	assert( app_ptr );
	// If the given word is empty, it's a perfect match. Otherwise, there is no match.
	return word.empty() ? app_ptr : nullptr;
}

// Set root to a new node corresponding to the empty trie.
Trie::Trie() : root( new Node { false, map<string, Node*>(), nullptr } ) {}

Trie::Trie( const initializer_list<string>& key_list ) : Trie() {
	try {
		insert( key_list );
	}
	catch ( bad_alloc& e ) {
		recursive_delete( root );
		cerr << e.what() << endl;
	}
}

Trie::Trie( const Trie& other ) : Trie() {
	assert( other.root );
	try {
		// The default Trie constructor already set root to a node.
		recursive_copy( root, other.root );
	}
	catch ( bad_alloc& e ) {
		recursive_delete( root );
		cerr << e.what() << endl;
	}
}

Trie::Trie( Trie&& other ) : Trie() {
	swap( *this, other );
}

Trie::~Trie() {
	recursive_delete( root );
}

Trie& Trie::operator=( Trie other ) {
	swap( *this, other );
	return *this;
}

bool Trie::empty( const string& prefix ) const {
	auto cp (prefix);
	const Node* prf_rt = prefix_match( root, cp );
	// Check if prefix root is null
	if ( !prf_rt ) return true;
	// It's empty if prf_rt is not a word and has no children.
	return !prf_rt->is_end && prf_rt->children.empty();
}

size_t Trie::size( const string& prefix ) const {
	auto cp (prefix);
	const Node* prf_rt = prefix_match( root, cp );
	if ( !prf_rt ) return size_t(0);
	size_t counter = 0;
	key_counter( prf_rt, counter );
	return counter;
}

Trie::iterator Trie::find( string key, bool is_prefix ) const {
	// Check if we need an exact match.
	if (!is_prefix) {
		// Elegantly handles both match and no match.
		return iterator(*this, exact_match( root, key ));
	}
	
	// In this case, we need only find a word that key is a prefix of.
	const Node* prf_rt = prefix_match( root, key );
	// If key is not a prefix of anything, there is no match.
	if ( !prf_rt ) return iterator(*this);

	// Move down the left of the tree until an is_end flag is encountered.
	while ( !prf_rt->is_end ) {
		prf_rt = prf_rt->children.begin()->second;
	}
	return iterator(*this, prf_rt);
}

Trie::iterator Trie::insert( string key ) {
	/*
	Note: inserting key at root, is the same
	as inserting reduced key at loc.
	The problem space has been reduced.
	*/
	Node* loc = approximate_match( root, key );
	assert( loc );
	/* INSERT KEY AT LOC */

	// If the key is now empty, simply set is_end to true.
	if ( key.empty() ) {
		loc->is_end = true;
		return iterator(*this, loc);
	}

	// TODO: handle the case when key is non-empty.
}

void Trie::insert( const initializer_list<std::string>& list ) {
	for ( const auto& str : list ) {
		insert( str );
	}
}

void Trie::erase( const string& key, bool is_prefix ) {

}

void Trie::erase( const initializer_list<std::string>& list ) {
	for ( const auto& str : list ) {
		erase( str );
	}
}

void Trie::clear() {
	// First delete everything.
	recursive_delete( root );
	// Then set root to default empty value.
	root = new Node { false, map<string, Node*>(), nullptr };
}

Trie::iterator::iterator( const Trie& t, const Node* p ) : tree(t), ptr(p) {}

Trie::iterator& Trie::iterator::operator++() {

}

Trie::iterator Trie::iterator::operator++(int) {
	auto temp(*this);
	++(*this);
	return temp;
}

Trie::iterator& Trie::iterator::operator--() {

}

Trie::iterator Trie::iterator::operator--(int) {
	auto temp(*this);
	--(*this);
	return temp;
}

string Trie::iterator::operator*() {

}

Trie::iterator Trie::begin() const {

}

Trie::iterator Trie::end() const {
	return iterator(*this, nullptr);
}

Trie::iterator Trie::begin( const string& prefix ) const {

}

Trie::iterator Trie::end( const string& prefix ) const {

}


Trie& Trie::operator+=( const Trie& rhs ) {
	assert( this != &rhs );
	for ( const auto& str : rhs ) {
		insert( str );
	}
}

Trie operator+( Trie lhs, const Trie& rhs ) {
	return lhs += rhs;
}

Trie& Trie::operator-=( const Trie& rhs ) {
	assert( this != &rhs );
	for ( const auto& str : rhs ) {
		erase( str );
	}
}

Trie operator-( Trie lhs, const Trie& rhs ) {
	return lhs -= rhs;
}

bool operator==( const Trie& lhs, const Trie& rhs ) {
	return Trie::are_equal( lhs.root, rhs.root );
}

bool operator!=( const Trie& lhs, const Trie& rhs ) {
	return !(lhs == rhs);
}

bool operator<( const Trie& lhs, const Trie& rhs ) {

}

bool operator>( const Trie& lhs, const Trie& rhs ) {
	return rhs < lhs;
}

bool operator<=( const Trie& lhs, const Trie& rhs ) {
	return !(rhs < lhs);
}

bool operator>=( const Trie& lhs, const Trie& rhs ) {
	return !(lhs < rhs);
}

ostream& operator<<( std::ostream& os, const Trie& tree ) {
	for ( const auto& str : tree ) {
		os << str << '\n';
	}
	return os;
}

bool operator==( const Trie::iterator& lhs, const Trie::iterator& rhs ) {
	// Performs element by element.
	return lhs == rhs;
}

bool operator!=( const Trie::iterator& lhs, const Trie::iterator& rhs ) {
	// Performs element by element.
	return lhs != rhs;
}