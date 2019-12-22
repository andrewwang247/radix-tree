#include "trie.h"
#include <iostream>
#include <vector>
#include <functional>
#include <fstream>
#include <exception>
#include <chrono>
#include <set>
using namespace std;
using namespace std::chrono;

using time_unit = milliseconds;

// Forward declarations for test cases.

bool Empty_Test();
bool Find_Test();

// Reads perf_word_list file into a vector reserved to num_perf_words.
vector<string> read_words( const string& perf_word_list, size_t num_perf_words );

// Prints (to cout) the number of time_units elapsed between start and finish.
void print_duration(time_point<chrono::_V2::system_clock, nanoseconds> start,
					time_point<chrono::_V2::system_clock, nanoseconds> finish );

// Forward declarations for perf tests.

set<string> get_set( const vector<string>& word_list );
Trie get_trie( const vector<string>& word_list );

int main() {
	vector< function<bool()> > test_cases {
		Empty_Test,
		Find_Test
	};

	// TODO: Design unit tests.
	
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
	//* Make sure that this announcement matches time_units.
	cout << "Time measured in milliseconds.\n";

	const auto master_list = read_words("words.txt", 466551);

	// Insertion perf

	set<string> word_set = get_set(master_list);
	Trie word_trie = get_trie(master_list);

	// TODO: Test performance of word_set and word_trie.
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

vector<string> read_words( const string& perf_word_list, size_t num_perf_words ) {
	ifstream fin (perf_word_list);
	if (!fin) throw runtime_error("Could not open words.txt");

	cout << "Importing words.txt...\n";
	vector<string> master_list;
	master_list.reserve(num_perf_words);
	for ( string word; fin >> word; ) {
		master_list.push_back(word);
	}
	fin.close();
}

void print_duration(time_point<chrono::_V2::system_clock, nanoseconds> start,
					time_point<chrono::_V2::system_clock, nanoseconds> finish ) {

	cout << "Time: " << duration_cast<time_unit>( finish - start ).count() << endl;
}

set<string> get_words( const vector<string>& word_list ) {
	cout << "Set Insertion...\n";
	set<string> word_set;
	auto start = high_resolution_clock::now();
	for ( const auto& str : word_list ) {
		word_set.insert(str);
	}
	auto finish = high_resolution_clock::now();
	print_duration( start, finish );
	return word_set; 
}

Trie get_trie( const vector<string>& word_list ) {
	cout << "Trie Insertion...\n";
	Trie word_trie;
	auto start = high_resolution_clock::now();
	for ( const auto& str : word_list ) {
		word_trie.insert(str);
	}
	auto finish = high_resolution_clock::now();
	print_duration( start, finish );
}