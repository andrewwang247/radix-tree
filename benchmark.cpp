#include "unit_test.h"
#include <iostream>
#include <memory>
using namespace std;

int main() {

	/* --- UNIT TESTING --- */

	// Add all unit tests to this vector.
	vector< unique_ptr<Unit_Test> > tests {
		unique_ptr<Unit_Test> ( new Default_Ctor_Test ),
		unique_ptr<Unit_Test> ( new Initializer_Ctor_Test ),
		unique_ptr<Unit_Test> ( new Copy_Ctor_Test ),
		unique_ptr<Unit_Test> ( new Empty_Test ),
		unique_ptr<Unit_Test> ( new Size_Test ),
		unique_ptr<Unit_Test> ( new Find_Test ),
		unique_ptr<Unit_Test> ( new Insert_Test ),
		unique_ptr<Unit_Test> ( new Erase_Test ),
		unique_ptr<Unit_Test> ( new Iterator_Test ),
		unique_ptr<Unit_Test> ( new Comparison_Test ),
		unique_ptr<Unit_Test> ( new Arithmetic_Test )
	};

	size_t passed = 0, failed = 0;
	for ( const auto& test : tests ) {
		if ( test->operator() ) {
			cout << "Passed.\n";
			++passed;
		} else {
			cout << "Failed.\n";
			++failed;
		}
	}
	cout << "--- Finished ---\n";
	cout << "Total: " << tests.size() << " test cases.\n";
	cout << "Passed " << passed << " and failed " << failed << " of them." << endl;

	/* --- PERFORMANCE TESTING --- */

}