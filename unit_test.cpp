#include "trie.h"
#include <vector>
#include <utility>
#include <variant>
#include <functional>
#include <exception>
using namespace std;

/**
 * The Unit_Test class defines a white box unit testing framework
 * for the Trie class for which Unit_Test is a friend.
 */
class Unit_Test {
private:
	
	/**
	 * The result of each individual test case will be one of these 3 types.
	 * @param bool: Comparison operators.
	 * @param Node*: Used for most Trie functions.
	 * @param string: Checking iterators.
	 */
	using Result = variant<bool, Trie::Node*, string>;

	/**
	 * Contains all test cases. The function returns a Result
	 * which is compared with the expected Result value. 
	 */
	vector< pair< function<Result()>, Result> > Tests;

	/**
	 * Checks for equality between two results.
	 */
	bool result_equality( Result r1, Result r2 ) {
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

public:

	/**
	 * Executes every test in the testing framework.
	 * Prints the result of the tests.
	 */
	void run() {
		size_t test_number = 1, passed = 0, failed = 0;
		for ( const auto& fnc_res : Tests ) {
			cout << "Executing Test " << test_number++ << "...\n";
			auto res = fnc_res.first();
			if ( result_equality( res, fnc_res.second ) ) {
				cout << "Passed.\n";
				++passed;
			} else {
				cout << "Failed.\n";
				++failed;
			}
		}

		cout << "Finished " << test_number - 1 << " test cases.\n";
		cout << "Passed " << passed << " and failed " << failed << " of them." << endl;
	}
};