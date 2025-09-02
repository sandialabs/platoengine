#include <gtest/gtest.h>

#include <algorithm>
#include <string>

#include "plato/utilities/CartesianProduct.hpp"

namespace plato::utilities::unittest
{
namespace
{
const auto kCartesianProduct = CartesianProduct{{2UL, 3UL}};
const auto kCartesianProductTwo = CartesianProduct{{2UL, 3UL, 2UL}};

}  // namespace

TEST(CartesianProduct, Begin)
{
    const auto tCartesianProduct = kCartesianProduct;
    const auto tResult = *tCartesianProduct.begin();
    EXPECT_EQ(tResult.front(), 0UL);
    EXPECT_EQ(tResult.back(), 0UL);
}

TEST(CartesianProduct, End)
{
    const auto tCartesianProduct = kCartesianProduct;
    const auto tCartesianProductIteratorEnd = tCartesianProduct.end();
    auto tCartesianProductIterator = tCartesianProduct.begin();
    for (unsigned int tCounter = 0; tCounter < 6; ++tCounter)
    {
        tCartesianProductIterator++;
    }
    EXPECT_EQ(tCartesianProductIterator, tCartesianProductIteratorEnd);
}

TEST(CartesianProduct, NotEquals)
{
    auto tCartesianProduct = kCartesianProductTwo;
    EXPECT_TRUE(tCartesianProduct.begin() != tCartesianProduct.end());
}

TEST(CartesianProduct, For)
{
    auto tCartesianProduct = kCartesianProduct;
    std::string tString = "";
    for (const auto& tProduct : tCartesianProduct)
    {
        tString += std::to_string(tProduct.front()) + std::to_string(tProduct.back());
    }
    constexpr auto tGold = std::string_view{"001001110212"};
    EXPECT_EQ(tGold, tString);
}

TEST(CartesianProduct, Transform)
{
    auto tCartesianProduct = kCartesianProduct;
    std::vector<std::size_t> tFirstIndexValues;
    tFirstIndexValues.reserve(12);
    std::transform(tCartesianProduct.begin(), tCartesianProduct.end(), std::back_inserter(tFirstIndexValues),
                   [](const auto aSizeVector) { return aSizeVector.front(); });
    const auto tGold = std::vector<std::size_t>{0, 1, 0, 1, 0, 1};
    EXPECT_EQ(tGold, tFirstIndexValues);
}

TEST(CartesianProduct, ForEach)
{
    auto tCartesianProduct = kCartesianProductTwo;
    auto tCount = 0U;
    std::for_each(tCartesianProduct.begin(), tCartesianProduct.end(),
                  [&tCount](const auto aSizeVector) { tCount += aSizeVector.size(); });
    const auto tGold = 2U * 3U * 2U * 3U;
    EXPECT_EQ(tCount, tGold);
}

TEST(CartesianProductIterator, PostIncrement)
{
    std::vector<std::size_t> tSizes = {2, 2};
    plato::utilities::CartesianProductIterator tIterator(tSizes);

    const auto tZeroZero = std::vector<std::size_t>{0, 0};
    const auto tResult = *tIterator;
    EXPECT_EQ(tResult, tZeroZero);
    auto tPreviousState = tIterator++;
    EXPECT_EQ(*tPreviousState, tZeroZero);

    const auto tOneZero = std::vector<std::size_t>{1, 0};
    EXPECT_EQ(*tIterator, tOneZero);
    tPreviousState = tIterator++;  /// 0 , 1
    EXPECT_EQ(*tPreviousState, tOneZero);
    const auto tZeroOne = std::vector<std::size_t>{0, 1};
    EXPECT_EQ(*tIterator, tZeroOne);

    tIterator++;  /// 1 , 1
    tIterator++;  /// end
    const auto tLastIsFirst = std::vector<std::size_t>{0, 0};
    EXPECT_EQ(*tIterator, tLastIsFirst);
}

}  // namespace plato::utilities::unittest
