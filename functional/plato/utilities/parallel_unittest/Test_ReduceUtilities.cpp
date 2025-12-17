#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>
#include <ranges>
#include <unordered_map>

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

TEST(ReduceUtilities, ConcatenateOverAllRanks)
{
    const auto tCommunicator = boost::mpi::communicator{};
    const auto tRank = tCommunicator.rank();
    constexpr auto tBaseSize = 2U;
    std::vector<long unsigned int> tValues(tBaseSize, tCommunicator.size() - tRank);

    const auto tConcatenated = concatenate_over_all_ranks(tValues, tCommunicator);

    auto tGold =
        std::views::iota(std::size_t{1}, static_cast<std::size_t>(tCommunicator.size() + 1)) | std::views::reverse |
        std::views::transform([](const auto tValue) { return std::vector<long unsigned int>(tBaseSize, tValue); }) |
        std::views::join | std::views::common;

    EXPECT_EQ(std::vector(tGold.begin(), tGold.end()), tConcatenated);
}

TEST(ReduceUtilities, ConcatenateOverAllRanksAndSort)
{
    const auto tCommunicator = boost::mpi::communicator{};
    const auto tRank = tCommunicator.rank();

    // Allocate 5 entries to each rank, except for rank 0. It will be empty because this was found to be a problematic
    // case.
    const auto tBaseSize = 5U;
    const auto tSize = tCommunicator.rank() == 0 ? 0U : tBaseSize;
    std::vector<long unsigned int> tIds(tSize, 0U);
    std::iota(tIds.begin(), tIds.end(), tRank * tBaseSize + 1U);

    const auto tMergedSorted = concatenate_over_all_ranks_and_sort(tIds, tCommunicator);

    auto tGold = std::vector<long unsigned int>(tBaseSize * (tCommunicator.size() - 1U));
    std::iota(tGold.begin(), tGold.end(), tBaseSize + 1U);

    EXPECT_EQ(tGold, tMergedSorted);
}

TEST(ReduceUtilities, ReduceMap)
{
    const auto tCommunicator = boost::mpi::communicator{};
    const auto tRank = tCommunicator.rank();

    std::unordered_map<int, double> tMap;
    tMap[tRank] = tRank;

    const auto tReducedMap = reduce_map(tMap, tCommunicator);

    ASSERT_EQ(tReducedMap.size(), tCommunicator.size());
    for (const auto& [tKey, tValue] : tReducedMap)
    {
        EXPECT_EQ(tKey, tValue);
    }
}

}  // namespace plato::utilities::parallel_unittest
