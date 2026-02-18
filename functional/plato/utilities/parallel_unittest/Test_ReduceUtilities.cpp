#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>
#include <unordered_map>

#include "plato/test_utilities/TestContext.hpp"
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

namespace
{
template <typename VectorType>
void test_reduce_on_vector_type(const test_utilities::TestContext& aTestContext, const VectorType aOptionalOffset = 0)
{
    const auto tCommunicator = boost::mpi::communicator{};
    const auto tRank = tCommunicator.rank();

    // Allocate 5 entries to each rank, except for rank 0. It will be empty because this was found to be a problematic
    // case.
    const auto tBaseSize = VectorType{5};
    const auto tSize = tCommunicator.rank() == 0 ? 0U : tBaseSize;
    std::vector<VectorType> tIds(tSize, 0U);
    std::iota(tIds.begin(), tIds.end(), tRank * tBaseSize + 1U + aOptionalOffset);

    const auto tMergedSorted = merge_on_all_ranks(tIds, tCommunicator);

    auto tGold = std::vector<VectorType>(tBaseSize * (tCommunicator.size() - 1U));
    std::iota(tGold.begin(), tGold.end(), tBaseSize + 1U + aOptionalOffset);

    EXPECT_EQ(tGold, tMergedSorted) << aTestContext;
}
}  // namespace

TEST(ReduceUtilities, MergeOnAllRanks)
{
    test_reduce_on_vector_type<long unsigned int>(TEST_CONTEXT("Reduce on long unsigned int"));
}

TEST(ReduceUtilities, MergeOnAllRanksDoubles)
{
    constexpr auto tOffset = double{1.1};
    test_reduce_on_vector_type<double>(TEST_CONTEXT("Reduce on long unsigned int"), tOffset);
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
