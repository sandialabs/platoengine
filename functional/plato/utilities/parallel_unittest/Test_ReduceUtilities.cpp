#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>

#include "plato/utilities/ReduceUtilities.hpp"

namespace plato::utilities::parallel_unittest
{

TEST(ReduceUtilities, UniqueVectorGather)
{
    const auto tCommunicator = boost::mpi::communicator{};
    const auto tRank = tCommunicator.rank();

    auto tVector = std::vector<double>(3, 0.0);
    std::iota(tVector.begin(), tVector.end(), tRank);

    const auto tResult = unique_vector_gather(tVector, tCommunicator);
    const auto tGold = std::vector<double>{0, 1, 2, 3, 4};
    EXPECT_EQ(tResult, tGold);
}

}  // namespace plato::utilities::parallel_unittest
