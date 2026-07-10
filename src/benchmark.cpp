/*
Copyright 2026. Andrew Wang.

Benchmarking for correctness and performance.
*/
#include <iomanip>
#include <ios>
#include <iostream>

#include "perf_test.h"
#include "unit_test.h"

using std::cout;
using std::fixed;
using std::ios_base;
using std::setprecision;

int main() {
  ios_base::sync_with_stdio(false);
  cout << setprecision(1) << fixed;

#ifdef DEBUG
  unit_test::run_all();
#endif

#ifdef NDEBUG
  perf_test::run_all();
#endif
}
