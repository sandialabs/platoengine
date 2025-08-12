#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <vector>

#include "plato/utilities/HashUtilities.hpp"

namespace plato::utilities::unittest
{
TEST(HashUtilities, HashDifferentContainerTypes)
{
    const auto tVector = std::vector{-1.5, 0.25, 2.125};
    const auto tVectorHash = hash_container(tVector);

    const auto tArray = std::array{-1.5, 0.25, 2.125};
    const auto tArrayHash = hash_container(tArray);

    EXPECT_EQ(tVectorHash, tArrayHash);
}

TEST(HashUtilities, HashPermutedContainer)
{
    auto tVector = std::vector{-2, 0, 2};
    const auto tVectorHash = hash_container(tVector);

    std::next_permutation(tVector.begin(), tVector.end());
    const auto tPermutedVectorHash = hash_container(tVector);

    EXPECT_NE(tVectorHash, tPermutedVectorHash);
}

}  // namespace plato::utilities::unittest
