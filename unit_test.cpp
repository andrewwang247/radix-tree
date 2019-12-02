#include "unit_test.h"
using namespace std;

Unit_Test::Result Unit_Test::test() const {
	throw runtime_error("Base class Unit_Test cannot be called. Use an inherited class.");
}

bool Unit_Test::operator()() const {
	Result actual = test();
	return result_equality( actual, answer );
}

bool Unit_Test::result_equality( Result r1, Result r2 ) {
	if ( holds_alternative<bool>(r1) && holds_alternative<bool>(r2) ) {
		return get<bool>(r1) == get<bool>(r2);
	} else if ( holds_alternative<Trie::Node*>(r1) && holds_alternative<Trie::Node*>(r2) ) {
		// Check for equality between Nodes.
	} else if ( holds_alternative<string>(r1) && holds_alternative<string>(r2) ) {
		return get<string>(r1) == get<string>(r2);
	}
	// Throw exception if types are a mismatch.
	throw runtime_error("Result types do not match.");
}