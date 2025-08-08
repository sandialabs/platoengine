#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>

#include "plato/utilities/MPIUtilities.hpp"

namespace plato::utilities::parallel_unittest
{

TEST(MPIUtilities, ExecuteOnRoot)
{
    constexpr auto tValue = 666;
    constexpr auto tOriginal = 42;

    const auto tLambda = [](int& aInteger) { aInteger = tValue; };
    const auto tCommunicator = boost::mpi::communicator{};

    auto tInteger = tOriginal;
    execute_on_root(tCommunicator, tLambda, tInteger);

    if (tCommunicator.rank() == 0)
    {
        EXPECT_EQ(tInteger, tValue);
    }
    else
    {
        EXPECT_EQ(tInteger, tOriginal);
    }
}
}  // namespace plato::utilities::parallel_unittest
