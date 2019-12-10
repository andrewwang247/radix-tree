#include "trie.h"
#include <iostream>
#include <vector>
#include <functional>
using namespace std;

// Forward declarations for test cases.

bool Empty_Test();
bool Find_Test();

int main() {
	vector< function<bool()> > test_cases {
		Empty_Test,
		Find_Test
	};
	
	cout << "--- EXECUTING UNIT TESTS ---\n";
	unsigned passed = 0;
	for ( auto& test : test_cases ) {
		if ( test() ) {
			++passed;
			cout << " passed\n";
		} else {
			cout << " failed\n";
		}
	}

	cout << "--- FINISHED UNIT TESTS ---\n";
	cout << "Results: " << passed << " out of " << test_cases.size() << endl;

	cout << "--- EXECUTING PERFORMANCE TEST ---\n";
	// TODO: Design performance tests.
}

bool Empty_Test() {
	cout << "Empty Test";
	Trie tr;
	if ( !tr.empty() ) return false;
	if ( !tr.empty("hello") ) return false;
	if ( tr.size() != 0 ) return false;
	if ( tr.size("world") != 0 ) return false;
	if ( tr.find("test") != tr.end() ) return false;
	return true;
}

bool Find_Test() {
	cout << "Find Test";
	Trie tr { "mahogany", "mahjong", "compute", "computer", "matrix", "math", "contaminate", "corn", "corner", "material", "mat", "maternal", "contain" };

	if ( tr.empty() ) return false;
	if ( tr.size() != 13 ) return false;
	if ( tr.size("ma") != 7 ) return false;

	auto exact_iter = tr.find("corn");
	if ( exact_iter == tr.end() || *exact_iter != "corn" ) return false;

	auto prf_iter = tr.find("mate", Trie::PREFIX_FLAG);
	if ( prf_iter == tr.end() || *prf_iter != "material" ) return false;

	auto missing_exact_iter = tr.find("testing");
	if ( missing_exact_iter != tr.end() ) return false;

	auto missing_prf_iter = tr.find("conk");
	if ( missing_prf_iter != tr.end() ) return false;

	return true;
}