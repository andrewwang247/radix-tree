#include "trie.h"
#include <algorithm>
#include <limits>
using namespace std;

void Trie::recursive_copy( Node* const rt, const Node* other ) {
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

Trie::Node* Trie::approximate_match( const Node* const rt, string& key ) {
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

Trie::Node* Trie::prefix_match( const Node* const rt, string& prf ) {
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

void Trie::key_counter( const Node* const rt, size_t& acc ) noexcept {
	assert( rt );
	// If root contains a word, increment the counter.
	if ( rt->is_end ) ++acc;
	// Recursively check for words in children
	for ( const auto& str_ptr_pair : rt->children ) {
		assert( str_ptr_pair.second );
		key_counter( str_ptr_pair.second, acc );
	}
}

Trie::Node* Trie::exact_match( const Node* const rt, string word ) {
	// First compute the approximate root.
	Node* app_ptr = approximate_match( rt, word );
	assert( app_ptr );
	// If the given word is empty, it's a perfect match. Otherwise, there is no match.
	return word.empty() ? app_ptr : nullptr;
}

bool Trie::are_equal( const Node* const rt_1, const Node* const rt_2 ) {
	assert( rt_1 && rt_2 );
	// Check is_end parameters match.
	if ( rt_1->is_end != rt_2->is_end ) return false;
	// Check that number of children are the same.
	if ( rt_1->children.size() != rt_2->children.size() ) return false;
	// Since the number of children match, we can iterate in parallel.
	auto it_1 = rt_2->children.cbegin();
	auto it_2 = rt_2->children.cbegin();
	while ( it_1 != rt_1->children.cend() ) {
		// Check that the strings on the branches match.
		if ( it_1->first != it_2->first ) return false;
		// Recursively check for equality.
		if ( !are_equal( it_1->second, it_2->second ) ) return false;
		++it_1;
		++it_2;
	}
	return true;
}

Trie::Node* Trie::first_key( const Node* rt ) {
	assert( rt );
	// Keep moving down the tree along the left side until is_end.
	while ( !rt->is_end ) {
		assert( !rt->children.empty() );
		rt = rt->children.begin()->second;
		assert( rt );
	}
	return const_cast<Node*>(rt);
}

Trie::Node* Trie::next_over( const Node* ptr ) {
	assert( ptr );

	// Go up once then keep going up until we can move right.
	auto par = ptr->parent;
	// Note that par->children cannot be empty since its a parent.
	while ( par && par->children.rbegin()->second == ptr ) {
		// Move up.
		ptr = par;
		par = par->parent;
	}

	// If par is null, there is nothing to the right. Return null
	if ( !par ) return nullptr;

	/*
	If par is non-null, the only way we broke out of the while
	loop is because ptr is not the right-most child.
	Thus, we want to find the child to the right of ptr.
	*/
	auto child_iter = value_find( par->children, ptr );
	assert( child_iter != par->children.end() );
	assert( next( child_iter ) != par->children.end() );
	++child_iter;
	auto rn = child_iter->second;
	assert( rn );

	// Return the smallest key rooted at rn.
	return first_key( rn );
}

string Trie::underlying_string( const Node* const ptr ) {

}

// Set root to a new node corresponding to the empty trie.
Trie::Trie() : root( new Node { false, map<string, Node*>(), nullptr } ) {}

Trie::Trie( const initializer_list<string>& key_list ) : Trie() {
	try {
		for_each( key_list.begin(), key_list.end(), insert );
	}
	catch ( bad_alloc& e ) {
		recursive_delete( root );
		throw e;
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
		throw e;
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
	const Node* const prf_rt = prefix_match( root, cp );
	// Check if prefix root is null
	if ( !prf_rt ) return true;
	// It's empty if prf_rt is not a word and has no children.
	return !prf_rt->is_end && prf_rt->children.empty();
}

size_t Trie::size( const string& prefix ) const {
	auto cp (prefix);
	const Node* const prf_rt = prefix_match( root, cp );
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

	// Find the first child key rooted at prt_rt.
	return iterator( *this, first_key( prf_rt ) );
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

void Trie::erase( const string& key, bool is_prefix ) {
	if ( is_prefix ) {
		string key_cp (key);
		auto prf_ptr = prefix_match( root, key_cp );
		// Delete everything under this prefix.
		recursive_delete( prf_ptr );
	} else {
		// Must remove exact key.
		auto match = exact_match( root, key );
		// If the key was not in the tree, just return.
		if ( !match ) return;

		// TODO: Remove the Node pointed to by match.
	}
}

void Trie::clear() {
	// Clear everything under root.
	for ( const auto& str_ptr_pair : root->children ) {
		recursive_delete( str_ptr_pair.second );
	}
	root->children.clear();
	// Reset the root node to delete empty string.
	root->is_end = false;
	assert( !root->parent );
}

Trie::iterator::iterator( const Trie& t, const Node* const p ) : tree(t), ptr(p) {}

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
	return underlying_string( ptr );
}

Trie::iterator::operator bool() const {
	return ptr != nullptr;
}

Trie::iterator Trie::begin() const {
	// Return the left most node that evaluates true.
	Node* ptr = root;
}

Trie::iterator Trie::end() const {
	return iterator(*this, nullptr);
}

Trie::iterator Trie::begin( const string& prefix ) const {
	// Find the first key that matches the given prefix.
	return find( prefix, PREFIX_FLAG );
}

Trie::iterator Trie::end( string prefix ) const {
	// Perform an approximate match.
	auto app_ptr = approximate_match( root, prefix );
	assert( app_ptr );

	if ( prefix.empty() ) {
		/*
		If prefix is empty, we've essentially found a prefix match.
		Therefore, nothing that's a child of this node works.
		*/
		return iterator( *this, next_over( app_ptr ) );
	} else {
		// Check if there are keys to the RIGHT of the remainder of prefix.
		if ( app_ptr->children.empty() || app_ptr->children.rbegin()->first < prefix ) {
			// If there are no children to the right of prefix, just return the next node.
			return iterator( *this, next_over( app_ptr ) );
		} else {
			// Otherwise, find the first node the right of prefix.
			for( const auto& str_ptr_pair : app_ptr->children ) {
				// Note that since we used approximate_match, none of the children can EQUAL prefix.
				if ( str_ptr_pair.first > prefix ) {
					// Return the left most node of this child.
					return iterator( *this, str_ptr_pair.second );
				}
			}

			// If we got here and did not find anything, something is wrong.
			assert( false );
			// This line so the code compiles with warning flags.
			return iterator( *this );
		}
	}
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