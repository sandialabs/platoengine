#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>

#include "plato/test_utilities/ParallelTestWrapper.hpp"

namespace plato::test_utilities::parallel_unittest
{
TEST(ParallelTestFunctionWrapper, MatchesSerialQuadratic)
{
    const auto tQuadraticFunction = [](const double x) { return x * x; };
    const auto tParallelQuadratic = test_utilities::ParallelTestFunctionWrapper<double, double>{tQuadraticFunction};

    const auto tComm = boost::mpi::communicator{};
    EXPECT_GT(tComm.size(), 1);
    constexpr auto tArg = 42.0;
    EXPECT_EQ(tQuadraticFunction(tArg), tParallelQuadratic(tArg, tComm));
}
}  // namespace plato::test_utilities::parallel_unittest
