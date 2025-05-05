#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>

#include "plato/utilities/ReduceUtilities.hpp"

namespace plato::utilities::parallel_unittest
{

TEST(ReduceUtilities, ReduceVector)
{
    const auto tCommunicator = boost::mpi::communicator{};
    const auto tRank = tCommunicator.rank();

    auto tVector = std::vector<double>(3, 0.0);
    tVector[tRank] = 1;

    const auto tResult = reduce_vector(tVector, tCommunicator);
    const auto tGold = std::vector<double>{1, 1, 1};
    EXPECT_EQ(tResult, tGold);
}

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

TEST(ReduceUtilities, MergeOnAllRanks)
{
    const auto tCommunicator = boost::mpi::communicator{};
    const auto tRank = tCommunicator.rank();

    // Allocate 5 entries to each rank, except for rank 0. It will be empty because this was found to be a problematic
    // case.
    const auto tBaseSize = 5U;
    const auto tSize = tCommunicator.rank() == 0 ? 0U : tBaseSize;
    std::vector<long unsigned int> tIds(tSize, 0U);
    std::iota(tIds.begin(), tIds.end(), tRank * tBaseSize + 1U);

    const auto tMergedSorted = merge_on_all_ranks(tIds, tCommunicator);

    auto tGold = std::vector<long unsigned int>(tBaseSize * (tCommunicator.size() - 1U));
    std::iota(tGold.begin(), tGold.end(), tBaseSize + 1U);

    EXPECT_EQ(tGold, tMergedSorted);
}

}  // namespace plato::utilities::parallel_unittest
