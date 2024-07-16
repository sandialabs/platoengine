
#include <gtest/gtest.h>

#include <cmath>

#include "plato/utilities/MultidimensionalRange.hpp"

namespace plato::utilities::unittest
{
TEST(MultidimensionalIterator, IteratorIncrement)
{
    constexpr auto tNumDimZero = int{2};
    constexpr auto tNumDimOne = int{3};
    auto tIterator = MultidimensionalIterator{std::make_tuple(tNumDimZero, tNumDimOne)};

    {
        const auto [tValueZero, tValueOne] = *tIterator;
        EXPECT_EQ(tValueZero, 0);
        EXPECT_EQ(tValueOne, 0);
    }
    ++tIterator;
    {
        const auto [tValueZero, tValueOne] = *tIterator;
        EXPECT_EQ(tValueZero, 0);
        EXPECT_EQ(tValueOne, 1);
    }
    ++tIterator;
    {
        const auto [tValueZero, tValueOne] = *tIterator;
        EXPECT_EQ(tValueZero, 0);
        EXPECT_EQ(tValueOne, 2);
    }
    ++tIterator;
    {
        const auto [tValueZero, tValueOne] = *tIterator;
        EXPECT_EQ(tValueZero, 1);
        EXPECT_EQ(tValueOne, 0);
    }
}

TEST(MultidimensionalIterator, IteratorEnd)
{
    constexpr auto tNumDimZero = int{2};
    constexpr auto tNumDimOne = int{3};
    using iterator_type = MultidimensionalIterator<int, int>;
    auto tIterator = MultidimensionalIterator{iterator_type::EndTag{}, std::make_tuple(tNumDimZero, tNumDimOne)};

    const auto [tValueZero, tValueOne] = *tIterator;
    EXPECT_EQ(tValueZero, tNumDimZero);
    EXPECT_EQ(tValueOne, 0);
}

TEST(MultidimensionalIterator, IteratorEquality)
{
    constexpr auto tNumDimZero = int{2};
    constexpr auto tNumDimOne = int{3};
    using iterator_type = MultidimensionalIterator<int, int>;
    auto tBeginIterator = MultidimensionalIterator{std::make_tuple(tNumDimZero, tNumDimOne)};
    auto tEndIterator = MultidimensionalIterator{iterator_type::EndTag{}, std::make_tuple(tNumDimZero, tNumDimOne)};

    EXPECT_NE(tBeginIterator, tEndIterator);

    // Increment enough times to reach the end
    for (int i = 0; i < tNumDimZero * tNumDimOne; ++i)
    {
        ++tBeginIterator;
    }

    EXPECT_EQ(tBeginIterator, tEndIterator);
}

TEST(MultidimensionalIterator, BeginAndEndFunctions)
{
    constexpr auto tNumDimZero = int{2};
    using iterator_type = MultidimensionalIterator<int>;
    const auto tBeginIteratorFromClass = MultidimensionalIterator{std::make_tuple(tNumDimZero)};
    const auto tEndIteratorFromClass = MultidimensionalIterator{iterator_type::EndTag{}, std::make_tuple(tNumDimZero)};

    const auto range = MultidimensionalRange{tNumDimZero};
    const auto tBeginIteratorFromRange = range.begin();
    const auto tEndIteratorFromRange = range.end();

    EXPECT_EQ(tBeginIteratorFromClass, tBeginIteratorFromRange);
    EXPECT_EQ(tEndIteratorFromClass, tEndIteratorFromRange);
}

TEST(MultidimensionalIterator, TwoDims)
{
    constexpr auto tNumDimZero = int{2};
    constexpr auto tNumDimOne = int{3};
    auto tManualCountZero = int{0};
    auto tManualCountOne = int{0};

    for (const auto [tIteratorCountZero, tIteratorCountOne] : MultidimensionalRange{tNumDimZero, tNumDimOne})
    {
        EXPECT_EQ(tManualCountZero, tIteratorCountZero);
        EXPECT_EQ(tManualCountOne, tIteratorCountOne);
        ++tManualCountOne;
        if (tManualCountOne == tNumDimOne)
        {
            ++tManualCountZero;
            tManualCountOne = 0;
        }
    }
}

TEST(MultidimensionalIterator, AllOf)
{
    const auto tOne = std::make_tuple(1, 2, 3);
    const auto tTwo = std::make_tuple(2, 3, 4);

    EXPECT_TRUE(allOf(tOne, tTwo, std::less{}));
    EXPECT_TRUE(allOf(tOne, tTwo, std::less_equal{}));
    EXPECT_FALSE(allOf(tOne, tTwo, std::greater{}));
    EXPECT_FALSE(allOf(tOne, tTwo, std::greater_equal{}));
    EXPECT_TRUE(allOf(tOne, tTwo, std::not_equal_to{}));
    EXPECT_FALSE(allOf(tOne, tTwo, std::equal_to{}));
    EXPECT_TRUE(allOf(tOne, tOne, std::equal_to{}));
}

}  // namespace plato::utilities::unittest
