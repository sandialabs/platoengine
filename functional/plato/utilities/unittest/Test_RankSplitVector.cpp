#include <gtest/gtest.h>

#include "plato/utilities/RankSplitVector.hpp"

namespace plato::utilities::unittest
{
namespace
{
constexpr auto kMPISize = int{3};
}  // namespace

TEST(RankSplitVector, NumElementsPerRank)
{
    {
        const auto [tQuotient, tRemainder] = detail::num_elements_per_rank(10, 2);
        EXPECT_EQ(tQuotient, 5);
        EXPECT_EQ(tRemainder, 0);
    }
    {
        const auto [tQuotient, tRemainder] = detail::num_elements_per_rank(10, 3);
        EXPECT_EQ(tQuotient, 3);
        EXPECT_EQ(tRemainder, 1);
    }
}

TEST(RankSplitVector, DividesEvenly)
{
    const auto tValues = std::vector<int>{1, 2, 3, 4, 5, 6};
    {
        const std::vector<int> tDistributedValues =
            rank_split_vector(tValues, RankNamedType{0}, SizeNamedType{kMPISize});
        EXPECT_EQ(tDistributedValues, (std::vector<int>{1, 2}));
    }
    {
        const std::vector<int> tDistributedValues =
            rank_split_vector(tValues, RankNamedType{1}, SizeNamedType{kMPISize});
        EXPECT_EQ(tDistributedValues, (std::vector<int>{3, 4}));
    }
    {
        const std::vector<int> tDistributedValues =
            rank_split_vector(tValues, RankNamedType{2}, SizeNamedType{kMPISize});
        EXPECT_EQ(tDistributedValues, (std::vector<int>{5, 6}));
    }
}

TEST(RankSplitVector, DividesUnevenlyOneRemaining)
{
    const auto tValues = std::vector<int>{1, 2, 3, 4, 5, 6, 7};
    {
        const std::vector<int> tDistributedValues =
            rank_split_vector(tValues, RankNamedType{0}, SizeNamedType{kMPISize});
        EXPECT_EQ(tDistributedValues, (std::vector<int>{1, 2, 7}));
    }
    {
        const std::vector<int> tDistributedValues =
            rank_split_vector(tValues, RankNamedType{1}, SizeNamedType{kMPISize});
        EXPECT_EQ(tDistributedValues, (std::vector<int>{3, 4}));
    }
    {
        const std::vector<int> tDistributedValues =
            rank_split_vector(tValues, RankNamedType{2}, SizeNamedType{kMPISize});
        EXPECT_EQ(tDistributedValues, (std::vector<int>{5, 6}));
    }
}

TEST(RankSplitVector, DividesUnevenlyTwoRemaining)
{
    const auto tValues = std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8};
    {
        const std::vector<int> tDistributedValues =
            rank_split_vector(tValues, RankNamedType{0}, SizeNamedType{kMPISize});
        EXPECT_EQ(tDistributedValues, (std::vector<int>{1, 2, 7}));
    }
    {
        const std::vector<int> tDistributedValues =
            rank_split_vector(tValues, RankNamedType{1}, SizeNamedType{kMPISize});
        EXPECT_EQ(tDistributedValues, (std::vector<int>{3, 4, 8}));
    }
    {
        const std::vector<int> tDistributedValues =
            rank_split_vector(tValues, RankNamedType{2}, SizeNamedType{kMPISize});
        EXPECT_EQ(tDistributedValues, (std::vector<int>{5, 6}));
    }
}

TEST(RankSplitVector, MoreRanksThanElements)
{
    const auto tValues = std::vector<int>{1, 2};
    {
        const std::vector<int> tDistributedValues =
            rank_split_vector(tValues, RankNamedType{0}, SizeNamedType{kMPISize});
        EXPECT_EQ(tDistributedValues, (std::vector<int>{1}));
    }
    {
        const std::vector<int> tDistributedValues =
            rank_split_vector(tValues, RankNamedType{1}, SizeNamedType{kMPISize});
        EXPECT_EQ(tDistributedValues, (std::vector<int>{2}));
    }
    {
        const std::vector<int> tDistributedValues =
            rank_split_vector(tValues, RankNamedType{2}, SizeNamedType{kMPISize});
        EXPECT_TRUE(tDistributedValues.empty());
    }
}
}  // namespace plato::utilities::unittest