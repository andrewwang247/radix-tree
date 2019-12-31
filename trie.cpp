#include "trie.h"
#include <algorithm>
#include <limits>
#include <unordered_set>
#include <stack>
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
	auto res = mismatch( prf.begin(), prf.end(), word.begin() );

	// If we reached the end of prf, it's a prefix.
	return res.first == prf.end();
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

void Trie::key_counter( const Node* const rt, size_t& acc ) {
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
	auto it_1 = rt_2->children.begin();
	auto it_2 = rt_2->children.begin();
	while ( it_1 != rt_1->children.end() ) {
		// Check that the strings on the branches match.
		if ( it_1->first != it_2->first ) return false;
		// Recursively check for equality.
		if ( !are_equal( it_1->second, it_2->second ) ) return false;
		++it_1;
		++it_2;
	}
	return true;
}

map<string, Trie::Node*>::const_iterator Trie::value_find( const map<string, Node*>& m, const Node* const val ) {
	for ( auto it = m.begin(); it != m.end(); ++it ) {
		if ( it->second == val ) return it;
	}
	return m.end();
}

Trie::Node* Trie::first_key( const Node* rt ) {
	assert( rt );

	// If rt has no children, nullptr.
	if ( rt->children.empty() ) return nullptr;

	// Keep moving down the tree along the left side until is_end.
	while ( !rt->is_end ) {
		// If rt is not an end, its children should not be empty.
		assert( !rt->children.empty() );
		rt = rt->children.begin()->second;
		assert( rt );
	}
	return const_cast<Node*>(rt);
}

Trie::Node* Trie::next_node( const Node* ptr ) {
	assert( ptr );

	// Go up once then keep going up until we can move right.
	auto par = ptr->parent;
	// Note that par->children cannot be empty since its a parent.
	assert( !par->children.empty() );
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

string Trie::underlying_string( const Node* ptr ) {
	assert( ptr );

	// As we move up, push string representations onto stack.
	stack<string> history;
	// Move up in trie until we get to root.
	auto par = ptr->parent;

	while( par ) {
		// We must be able to find ptr in par->children.
		auto iter = value_find( par->children, ptr );
		assert( iter != par->children.end() );

		// Push the string representation onto the stack and go up.
		history.push( iter->first );
		ptr = par;
		par = par->parent;
	}

	// If par is root, then ptr must be root. Concatenate strings in reverse.
	string str = "";
	while ( !history.empty() ) {
		str += history.top();
		history.pop();
	}

	return str;
}

bool Trie::check_invariant( const Node* const root ) {
	// Check that root is non-null.
	if ( !root ) return false;
	unordered_set<char> characters;

	// Check validity of children.
	for( const auto& str_ptr_pair : root->children ) {
		// No null nodes in children tree.
		if ( !str_ptr_pair.second ) return false;
		// Ensure that its parent is root.
		if ( str_ptr_pair.second->parent != root ) return false;
		// Make sure string is not empty.
		if ( str_ptr_pair.first.empty() ) return false;
		// Check that string does not share a prefix with other children.
		// We only really need to check first char.
		if ( characters.find( str_ptr_pair.first.front() ) != characters.end() ) return false;
		characters.insert( str_ptr_pair.first.front() );
		// Recursively check child node.
		if ( !check_invariant( str_ptr_pair.second ) ) return false;
	}

	// If root passes every single check, the tree is valid.
	return true;
}

// Set root to a new node corresponding to the empty trie.
Trie::Trie() : root( new Node { false, nullptr, map<string, Node*>() } ) {
	assert( check_invariant(root) );
}

Trie::Trie( const initializer_list<string>& key_list ) : Trie() {
	try {
		for ( auto& key : key_list ) {
			insert(key);
		}
	}
	catch ( bad_alloc& e ) {
		recursive_delete( root );
		throw e;
	}
	assert( check_invariant(root) );
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
	assert( check_invariant(root) );
}

Trie::Trie( Trie&& other ) : Trie() {
	swap( *this, other );
	assert( check_invariant(root) );
}

Trie::~Trie() {
	recursive_delete( root );
}

Trie& Trie::operator=( Trie other ) {
	swap( *this, other );
	assert( check_invariant(root) );
	return *this;
}

bool Trie::empty( string prefix ) const {
	const Node* const prf_rt = prefix_match( root, prefix );
	// Check if prefix root is null
	if ( !prf_rt ) return true;
	// It's empty if prf_rt is not a word and has no children.
	assert( check_invariant(root) );
	return !prf_rt->is_end && prf_rt->children.empty();
}

size_t Trie::size( string prefix ) const {
	const Node* const prf_rt = prefix_match( root, prefix );
	if ( !prf_rt ) return size_t(0);
	size_t counter = 0;
	key_counter( prf_rt, counter );

	assert( check_invariant(root) );
	return counter;
}

Trie::iterator Trie::find( string key, bool is_prefix ) const {
	// Check if we need an exact match.
	if (!is_prefix) {
		// Elegantly handles both match and no match.
		return iterator( exact_match( root, key ));
	}
	
	// In this case, we need only find a word that key is a prefix of.
	const Node* prf_rt = prefix_match( root, key );
	// If key is not a prefix of anything, there is no match.
	if ( !prf_rt ) return iterator();

	// Find the first child key rooted at prt_rt.
	assert( check_invariant(root) );
	// If key is empty and prf_rt is and end node, then it is the "first key".
	return key.empty() && prf_rt->is_end ? iterator( prf_rt ) : iterator( first_key( prf_rt ) );
}

Trie::iterator Trie::insert( string key ) {
	/*
	Note: inserting key at root, is the same
	as inserting reduced key at loc.
	The problem space has been reduced.
	*/
	auto loc = approximate_match( root, key );
	assert( loc );
	/* INSERT KEY AT LOC */

	// If the key is now empty, simply set is_end to true.
	if ( key.empty() ) {
		loc->is_end = true;
		assert( check_invariant(root) );
		return iterator( loc );
	}

	/*
	At this point, the key is non-empty.
	If loc has no children, then just make a child.
	*/
	if ( loc->children.empty() ) {
		loc->children[key] = new Node { true, loc, map<string, Node*>() };
		assert( check_invariant(root) );
		return iterator( loc->children[key]);
	}

	// Check children of loc for shared prefixes.
	for ( const auto& str_ptr_pair : loc->children ) {
		string child_str = str_ptr_pair.first;
		assert( !child_str.empty() );

		// Check when the first letter matches.
		if ( child_str.front() == key.front() ) {
			// Use mismatch to compute the spot where the prefix fails.
			auto iter_pair = mismatch( key.begin(), key.end(), child_str.begin() );
			// Extract the common prefix and unique postfixes of key and child.
			string common ( key.begin(), iter_pair.first );
			string post_key ( iter_pair.first, key.end() );
			string post_child ( iter_pair.second, child_str.end() );
			// If remaining key's prefix can match a child, then approximate_match failed.
			assert( !post_child.empty() );

			// Create a child for the common part. junction's parent is set.
			auto junction = new Node{ post_key.empty(), loc, map<string, Node*>() };
			// Add junction to loc under common.
			loc->children[ common ] = junction;
			// loc child is added to junction's children map.
			junction->children[ post_child ] = loc->children[ child_str ];
			// The original child's parent pointer is set to junction.
			loc->children[ child_str ]->parent = junction;
			// Remove child_str from loc child map.
			loc->children.erase( child_str );

			if ( !post_key.empty() ) {
				// Add an additional node for the split.
				auto key_node = new Node{ true, junction, map<string, Node*>() };
				junction->children[ post_key ] = key_node;

				assert( check_invariant(root) );
				return iterator(key_node);
			} else {
				assert( check_invariant(root) );
				return iterator(junction);
			}
		}
	}

	// If there are no shared prefixes, then simply make a new node under loc.
	auto key_node = new Node { true, loc, map<string, Node*>() };
	loc->children[key] = key_node;
	assert( check_invariant(root) );
	return iterator( key_node );
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

		match->is_end = false;

		// Check for special case if match is the root of the tree.
		if ( match == root ) {
			// If key was non empty, then exact_mach failed.
			assert( key.empty() );
			assert( check_invariant(root) );
			return;
		}
		// Now we can assume match is a non-root node.

		// If match was a leaf node, it can be removed.
		if ( match->children.empty() ) {
			auto par = match->parent;
			delete match;
			par->children.erase(key);
		} else if ( match->children.size() == 1 ) {
			// If match had only one child, the child joins with match's parent.
			auto child_iter_pair = match->children.begin();
			string child_str = child_iter_pair->first;
			assert( child_iter_pair->second );

			auto par = match->parent;
			assert( par ); // Non-root assumption.
			auto parent_iter_pair = value_find( par->children, match );
			string parent_str = parent_iter_pair->first;

			// Add a new connection to the map.
			string modified_str = parent_str + child_str;
			par->children[modified_str] = child_iter_pair->second;
			child_iter_pair->second->parent = par;

			// Clean up the old connection.
			par->children.erase( parent_iter_pair->first );
			delete match;
		}

		assert( check_invariant(root) );
		// In all other cases, we don't want to remove match.
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
	assert( check_invariant(root) );
}

Trie::iterator::iterator( const Node* const p ) : ptr(p) {}

Trie::iterator& Trie::iterator::operator=( iterator other ) {
	ptr = other.ptr;
	return *this;
}

Trie::iterator& Trie::iterator::operator++() {
	// If the ptr has children, return the first child.
	// Otherwise, return the next node that isn't a child.
	// Elegantly handles the case when the returned value is nullptr.
	ptr = ptr->children.empty() ? next_node(ptr) : first_key(ptr);
	return *this;
}

Trie::iterator Trie::iterator::operator++(int) {
	auto temp(*this);
	++(*this);
	return temp;
}

string Trie::iterator::operator*() {
	return underlying_string( ptr );
}

Trie::iterator::operator bool() const {
	return ptr != nullptr;
}

Trie::iterator Trie::begin() const {
	return iterator( first_key( root ) );
}

Trie::iterator Trie::end() const {
	return iterator( nullptr );
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
		return iterator( next_node( app_ptr ) );
	} else {
		// Check if there are keys to the RIGHT of the remainder of prefix.
		if ( app_ptr->children.empty() || app_ptr->children.rbegin()->first < prefix ) {
			// If there are no children to the right of prefix, just return the next node.
			return iterator( next_node( app_ptr ) );
		} else {
			// Otherwise, find the first node the right of prefix.
			for( const auto& str_ptr_pair : app_ptr->children ) {
				// Note that since we used approximate_match, none of the children can EQUAL prefix.
				if ( str_ptr_pair.first > prefix ) {
					// Return the left most node of this child.
					return iterator( str_ptr_pair.second );
				}
			}

			// If we got here and did not find anything, something is wrong.
			assert( false );
			// This line so the code compiles with warning flags.
			return iterator();
		}
	}
}

Trie& Trie::operator+=( const Trie& rhs ) {
	assert( this != &rhs );
	for ( const string& key : rhs ) {
		this->insert(key);
	}
	assert( check_invariant(root) );
	return *this;
}

Trie operator+( Trie lhs, const Trie& rhs ) {
	return lhs += rhs;
}

Trie& Trie::operator-=( const Trie& rhs ) {
	assert( this != &rhs );
	for ( const string& key : rhs ) {
		this->erase(key);
	}
	assert( check_invariant(root) );
	return *this;
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
	for ( const string& word : lhs ) {
		if ( rhs.find(word) == rhs.end() ) return false;
	}
	return lhs.size() < rhs.size();
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
	return lhs.ptr == rhs.ptr;
}

bool operator!=( const Trie::iterator& lhs, const Trie::iterator& rhs ) {
	// Performs element by element.
	return lhs.ptr != rhs.ptr;
}