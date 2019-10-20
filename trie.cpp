#include "trie.h"
using namespace std;

// TODO: helper function that returns the pointer at the head of a given prefix parameter.
// TODO: helper function that, given a pointer to a node, constructs the corresponding string.

Trie::Trie() : root( nullptr ) {}

Trie::Trie( const initializer_list<string>& key_list ) {

}

// Templated iterator constructor implemented in header.

Trie::Trie( const Trie& other ) {

}

Trie::Trie( Trie&& other ) : Trie() {
	swap( *this, other );
}

/**
 * Helper function
 * Recursively deletes all nodes that are children
 * of root as well as root itself.
 */
void recursive_delete( Node* root ) {
	if ( !root ) return;
	// Recursively delete all children.
	for ( auto str_ptr_pair : root->children ) {
		recursive_delete( str_ptr_pair.second );
	}
	delete root;
}

Trie::~Trie() {
	recursive_delete( root );
}

Trie& Trie::operator=( Trie other ) {
	swap( *this, other );
	return *this;
}

bool Trie::empty( const string& prefix ) const {

}

size_t Trie::size( const string& prefix ) const {

}

bool Trie::contains( const string& key, bool is_prefix ) const {

}

bool Trie::insert( const string& key ) {

}

size_t Trie::erase( const string& key, bool is_prefix ) {

}

void Trie::clear() {

}

Trie::iterator Trie::begin() {

}

Trie::iterator Trie::end() {
	return iterator(*this, nullptr);
}

Trie::const_iterator Trie::begin() const {

}

Trie::const_iterator Trie::end() const {
	return const_iterator(*this, nullptr);
}


Trie& Trie::operator+=( const Trie& rhs ) {

}

Trie operator+( Trie lhs, const Trie& rhs ) {
	return lhs += rhs;
}

Trie& Trie::operator-=( const Trie& rhs ) {

}

Trie operator-( Trie lhs, const Trie& rhs ) {
	return lhs -= rhs;
}

bool operator==( const Trie& lhs, const Trie& rhs ) {

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