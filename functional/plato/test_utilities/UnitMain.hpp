#ifndef PLATO_TEST_UTILITIES_UNITMAIN
#define PLATO_TEST_UTILITIES_UNITMAIN

namespace plato::test_utilities
{
/// @brief A unit test main for gtest.
///
/// Call this from any gtest unit tester. It calls the appropriate
/// gtest functions as well as initializing MPI and Kokkos.
int unit_main(int argc, char **argv);

/// @brief An MPI-based parallel unit test main for gtest.
///
/// Call this from any gtest unit tester to run MPI-based parallel tests. It calls the appropriate
/// gtest functions as well as initializing MPI and Kokkos.
///
/// The parallel behavior is to first check the number of ranks with which this was launched.
/// If more than one, it continues as normal, running the unit test suite in parallel.
/// If one, then @a aNumRanks new child processes are spawned, which then run the unit
/// test suite in parallel. The purpose of this behavior is to simplify usage in CI, so
/// that no special launch commands are needed, while also allowing the use of different
/// number of ranks during development.
int parallel_unit_main(int argc, char **argv, unsigned int aNumRanks);
}  // namespace plato::test_utilities

#endif
