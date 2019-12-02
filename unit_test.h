#pragma once
#include "trie.h"
#include <vector>
#include <variant>
#include <exception>

/**
 * The Unit_Test class defines a white box unit testing framework
 * for the Trie class for which Unit_Test is a friend.
 */
class Unit_Test {
protected:

	/**
	 * The result of each individual test case will be one of these 3 types.
	 * @param bool: Comparison operators.
	 * @param Node*: Used for most Trie functions.
	 * @param string: Checking iterators.
	 */
	using Result = std::variant<bool, Trie::Node*, std::string>;

private:

	/**
	 * The test case for this particular unit test. Should return
	 * one of the possible result types as a variant.
	 * This function should be overridden in each test case.
	 * Announces that the test is being run to cout.
	 */
	virtual Result test() const;

	/**
	 * Stores the actual answer for this particular test case.
	 */
	Result answer;

	/**
	 * Checks for equality between two results.
	 */
	static bool result_equality( Result r1, Result r2 );

public:

	/**
	 * Base class should not do anything to member variables.
	 * Override this when inheriting.
	 */
	Unit_Test() = default;

	/**
	 * Executes the test case and prints the result.
	 */
	bool operator()() const;
};

class Default_Ctor_Test : public Unit_Test {
public:
	Default_Ctor_Test();
private:
	Result test() const override;
};

class Initializer_Ctor_Test : public Unit_Test {
public:
	Initializer_Ctor_Test();
private:
	Result test() const override;
};

class Copy_Ctor_Test : public Unit_Test {
public:
	Copy_Ctor_Test();
private:
	Result test() const override;
};

class Empty_Test : public Unit_Test {
public:
	Empty_Test();
private:
	Result test() const override;
};

class Size_Test : public Unit_Test {
public:
	Size_Test();
private:
	Result test() const override;
};

class Find_Test : public Unit_Test {
public:
	Find_Test();
private:
	Result test() const override;
};

class Insert_Test : public Unit_Test {
public:
	Insert_Test();
private:
	Result test() const override;
};

class Erase_Test : public Unit_Test {
public:
	Erase_Test();
private:
	Result test() const override;
};

class Iterator_Test : public Unit_Test {
public:
	Iterator_Test();
private:
	Result test() const override;
};

class Comparison_Test : public Unit_Test {
public:
	Comparison_Test();
private:
	Result test() const override;
};

class Arithmetic_Test : public Unit_Test {
public:
	Arithmetic_Test();
private:
	Result test() const override;
};
