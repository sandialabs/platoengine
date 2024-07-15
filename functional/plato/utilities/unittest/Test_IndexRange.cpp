#include <gtest/gtest.h>

#include "plato/utilities/IndexRange.hpp"

namespace plato::utilities::unittest
{
TEST(IndexRange, Iterator)
{
    using IndexType = unsigned;
    constexpr auto tDimensions = IndexType{42};
    using IteratorType = IndexIterator<IndexType>;

    constexpr auto tBegin = IteratorType{tDimensions};
    constexpr auto tEnd = IteratorType{typename IteratorType::EndTag{}, tDimensions};

    static_assert(std::is_same_v<IndexType, IteratorType::value_type>);
    auto tIterator = tBegin;
    for (IndexType tIndex = 0; tIndex < tDimensions; ++tIndex)
    {
        EXPECT_EQ(tIndex, *tIterator);
        EXPECT_NE(tIterator, tEnd);
        ++tIterator;
    }
    EXPECT_EQ(tIterator, tEnd);
}

TEST(IndexRange, Range)
{
    using IndexType = int;
    constexpr auto tDimensions = IndexType{42};
    auto tExpectedIndex = 0;
    for (const auto tIndex : IndexRange{tDimensions})
    {
        static_assert(std::is_same_v<decltype(tIndex), const IndexType>);
        EXPECT_EQ(tIndex, tExpectedIndex);
        ++tExpectedIndex;
    }
}

}  // namespace plato::utilities::unittest
