#include "trie.h"
#include <algorithm>
using namespace std;

Trie::Trie() : root( nullptr ) {}

Trie::Trie( const initializer_list<string>& key_list ) : Trie() {
	try {
		insert( key_list );
	}
	catch ( bad_alloc& e ) {
		Helper::recursive_delete( root );
		cerr << e.what() << endl;
	}
}

// Templated iterator constructor implemented in header

Trie::Trie( const Trie& other ) : Trie() {
	if ( !other.root ) return;
	try {
		root = new Node;
		Helper::recursive_copy( root, other.root );
	}
	catch ( bad_alloc& e ) {
		Helper::recursive_delete( root );
		cerr << e.what() << endl;
	}
}

Trie::Trie( Trie&& other ) : Trie() {
	swap( *this, other );
}

Trie::~Trie() {
	Helper::recursive_delete( root );
}

Trie& Trie::operator=( Trie other ) {
	swap( *this, other );
	return *this;
}

bool Trie::empty( const string& prefix ) const {

}

size_t Trie::size( const string& prefix ) const {

}

Trie::iterator Trie::find( const string& key, bool is_prefix ) const {

}

Trie::iterator Trie::insert( const string& key ) {

}

// Templated insert function implemented in header

void Trie::insert( const initializer_list<std::string>& list ) {
	for ( const auto& str : list ) {
		insert( str );
	}
}

Trie::iterator Trie::erase( const string& key ) {

}

// Templated erase function implemented in header

void Trie::erase( const initializer_list<std::string>& list ) {
	for ( const auto& str : list ) {
		erase( str );
	}
}

void Trie::clear() {
	Helper::recursive_delete( root );
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
	for ( const auto str : tree ) {
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