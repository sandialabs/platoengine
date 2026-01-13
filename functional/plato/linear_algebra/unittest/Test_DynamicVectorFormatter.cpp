#include <gtest/gtest.h>

#include <format>
#include <numeric>

#include "plato/linear_algebra/DynamicVectorFormatter.hpp"

namespace plato::linear_algebra::unittest
{
TEST(DynamicVectorFormatter, ParseInt)
{
    {
        static constexpr auto tTest = std::string{"123"};
        static_assert(detail::to_int(tTest.begin(), tTest.end()) == std::size_t{123});
    }
    {
        static constexpr auto tTest = std::string{"1a3"};
        static_assert(detail::to_int(tTest.begin(), tTest.end()) == std::nullopt);
    }
}

TEST(DynamicVectorFormatter, Format)
{
    const auto tVector = DynamicVector{1, 2, 3};
    const auto tFormatOutput = std::format("{}", tVector);
    const auto tExpected = std::string{"1, 2, 3"};
    EXPECT_EQ(tFormatOutput, tExpected);
}

TEST(DynamicVectorFormatter, FormatZeroLength)
{
    {
        const auto tVector = DynamicVector<int>{};
        const auto tFormatOutput = std::format("{}", tVector);
        EXPECT_TRUE(tFormatOutput.empty());
    }
    {
        const auto tVector = DynamicVector<int>{1, 2, 3};
        const auto tFormatOutput = std::format("{:0}", tVector);
        EXPECT_TRUE(tFormatOutput.empty());
    }
}

TEST(DynamicVectorFormatter, FormatSingleEntry)
{
    {
        const auto tVector = DynamicVector<int>{67};
        EXPECT_EQ(std::format("{}", tVector), std::to_string(tVector.stdVector().front()));
    }
    {
        const auto tVector = DynamicVector<int>{42, 67, 88, 91};
        EXPECT_EQ(std::format("{:1}", tVector), std::to_string(tVector.stdVector().front()) + ", ...");
    }
}

TEST(DynamicVectorFormatter, FormatLimit)
{
    constexpr auto tEntry = 3;
    constexpr auto tMaximumEntries = 10U;
    const auto tVector = DynamicVector(tMaximumEntries + 2, tEntry);
    const auto tFormatOutput = std::format("{:10}", tVector);

    const auto tToExpected = std::views::iota(0U, tMaximumEntries) |
                             std::views::transform([](const auto&) { return std::to_string(tEntry) + ", "; }) |
                             std::views::common;
    auto tExpected = std::accumulate(tToExpected.begin(), tToExpected.end(), std::string{});
    tExpected += "...";

    EXPECT_EQ(tFormatOutput, tExpected);
}

TEST(DynamicVectorFormatter, FormatWithSpec)
{
    const auto tVector = DynamicVector{1.12345, 2.12345, 3.12345};
    {
        const auto tFormatOutput = std::format("{::3.2f}", tVector);
        const auto tExpected = std::string{"1.12, 2.12, 3.12"};
        EXPECT_EQ(tFormatOutput, tExpected);
    }
    {
        const auto tFormatOutput = std::format("{::3.2f}", tVector);
        const auto tExpected = std::string{"1.12, 2.12, 3.12"};
        EXPECT_EQ(tFormatOutput, tExpected);
    }
    {
        const auto tFormatOutput = std::format("{:3:3.2f}", tVector);
        const auto tExpected = std::string{"1.12, 2.12, 3.12"};
        EXPECT_EQ(tFormatOutput, tExpected);
    }
}

}  // namespace plato::linear_algebra::unittest
