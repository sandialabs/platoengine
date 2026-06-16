#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>

#include "plato/test_utilities/ParallelTestWrapper.hpp"

namespace plato::test_utilities::parallel_unittest
{
TEST(ParallelTestFunctionWrapper, MatchesSerialQuadratic)
{
    const auto tQuadraticFunction = [](const double aX) { return aX * aX; };
    const auto tParallelQuadratic = test_utilities::ParallelTestFunctionWrapper<double, double>{tQuadraticFunction};

    const auto tCommunicator = boost::mpi::communicator{};
    EXPECT_GT(tCommunicator.size(), 1);
    constexpr auto tArg = 42.0;
    EXPECT_EQ(tQuadraticFunction(tArg), tParallelQuadratic(tArg, tCommunicator));
}
}  // namespace plato::test_utilities::parallel_unittest
