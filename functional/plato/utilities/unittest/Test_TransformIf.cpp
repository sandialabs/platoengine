#include <gtest/gtest.h>

#include <algorithm>
#include <iterator>
#include <string>

#include "plato/utilities/TransformIf.hpp"

namespace plato::utilities::unittest
{

TEST(TransformIf, SquarePositiveInts)
{
    const auto tInts = std::vector{-1, 0, 2, 3, -2};
    const auto tIsPositive = [](const int aValue) { return aValue > 0; };
    auto tSquaredPositiveInts = std::vector<int>{};
    auto tBeginIterator = std::back_inserter(tSquaredPositiveInts);
    auto tEndIterator = transform_if(
        tInts, tBeginIterator, [](const int aValue) { return aValue * aValue; }, tIsPositive);
    const auto tNumPositiveInts = std::count_if(tInts.cbegin(), tInts.cend(), tIsPositive);
    ASSERT_EQ(tSquaredPositiveInts.size(), tNumPositiveInts);
    EXPECT_EQ(tSquaredPositiveInts.front(), 4);
    EXPECT_EQ(tSquaredPositiveInts.back(), 9);

    // Check that the iterator is still valid:
    tEndIterator = 42;
    ASSERT_EQ(tSquaredPositiveInts.size(), tNumPositiveInts + 1);
    EXPECT_EQ(tSquaredPositiveInts.back(), 42);
}

TEST(TransformIf, MapSum)
{
    const auto tIntMap = std::map<int, int>{{3, 5}, {4, -5}, {0, 4}, {2, 3}, {-10, -1}};
    const auto tSumIsPositive = [](const std::pair<int, int> aInts) { return (aInts.first + aInts.second) > 0; };
    auto tResult = std::vector<int>{};
    transform_if(
        tIntMap, std::back_inserter(tResult),
        [](const std::pair<int, int> aInts) { return aInts.first + aInts.second; }, tSumIsPositive);
    const auto tNumPositiveInts = std::count_if(tIntMap.cbegin(), tIntMap.cend(), tSumIsPositive);
    ASSERT_EQ(tResult.size(), tNumPositiveInts);
    // Map gets sorted by the first int, so the order is different than what is passed to the ctor
    EXPECT_EQ(tResult.at(0), 4);
    EXPECT_EQ(tResult.at(1), 5);
    EXPECT_EQ(tResult.at(2), 8);
}

TEST(TransformIf, ExistingRange)
{
    const auto tInts = std::vector{2, -3, 2, -5, -2};
    const auto tIsNegative = [](const int aValue) { return aValue < 0; };
    const auto tNumNegativeInts = std::count_if(tInts.cbegin(), tInts.cend(), tIsNegative);
    auto tStrings = std::vector<std::string>(tNumNegativeInts, "");
    transform_if(
        tInts, tStrings.begin(), [](const int aValue) { return std::to_string(aValue); }, tIsNegative);

    EXPECT_EQ(tStrings.at(0), "-3");
    EXPECT_EQ(tStrings.at(1), "-5");
    EXPECT_EQ(tStrings.at(2), "-2");
}

}  // namespace plato::utilities::unittest
