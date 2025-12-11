#include <gtest/gtest.h>

#include <ranges>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/utilities/MultiVectorView.hpp"
#include "plato/utilities/SingleDimensionMultiVectorView.hpp"

namespace plato::utilities::unittest
{
namespace
{
constexpr auto kDimensions = std::size_t{3};
constexpr auto kLength = std::size_t{5};

[[nodiscard]] auto test_vector()
{
    auto tEntryGenerator = std::views::iota(0U, kLength * kDimensions) | std::views::common;
    return std::vector(tEntryGenerator.begin(), tEntryGenerator.end());
}

template <typename CheckFunction>
void check_iterator_operations(const CheckFunction& aCheckFunction, const test_utilities::TestContext& aTestContext)
{
    auto tVector = test_vector();
    auto tMultiVectorView = MultiVectorView(tVector, kDimensions);

    for (const auto tDimensionIndex : std::views::iota(0U, kDimensions))
    {
        auto tIterator = SingleDimensionMultiVectorView<decltype(tVector)>::Iterator{tMultiVectorView, tDimensionIndex};
        aCheckFunction(tIterator, tDimensionIndex, tVector, aTestContext);
    }
}

[[nodiscard]] auto copy_random_access_range(const std::ranges::random_access_range auto& aRange)
{
    auto tResult = std::vector<unsigned long>();
    std::ranges::copy(aRange, std::back_inserter(tResult));
    return tResult;
}
}  // namespace

TEST(SingleDimensionMultiVectorView, IteratorConstDereference)
{
    check_iterator_operations([](const auto& aIterator, const auto aDimensionIndex, const auto& aBaseVector,
                                 const test_utilities::TestContext& aTestContext)
                              { EXPECT_EQ(*aIterator, aBaseVector.at(aDimensionIndex)) << aTestContext; },
                              TEST_CONTEXT("Const dereference"));
}

TEST(SingleDimensionMultiVectorView, IteratorMutableDereference)
{
    check_iterator_operations(
        [](auto aIterator, const auto aDimensionIndex, const auto& aBaseVector,
           const test_utilities::TestContext& aTestContext)
        {
            EXPECT_EQ(*aIterator, aBaseVector.at(aDimensionIndex)) << aTestContext;
            constexpr auto tNewValue = 42U;
            *aIterator = tNewValue;
            EXPECT_EQ(*aIterator, tNewValue) << aTestContext;
        },
        TEST_CONTEXT("Mutable dereference"));
}

TEST(SingleDimensionMultiVectorView, IteratorPreIncrement)
{
    check_iterator_operations(
        [](auto aIterator, const auto aDimensionIndex, const auto& aBaseVector,
           const test_utilities::TestContext& aTestContext)
        {
            ++aIterator;
            EXPECT_EQ(*aIterator, aBaseVector.at(aDimensionIndex + kDimensions)) << aTestContext;

            ++aIterator;
            EXPECT_EQ(*aIterator, aBaseVector.at(aDimensionIndex + 2 * kDimensions)) << aTestContext;

            [[maybe_unused]] const auto tNewIterat = aIterator;
        },
        TEST_CONTEXT("Pre-increment"));
}

TEST(SingleDimensionMultiVectorView, IteratorPostIncrement)
{
    check_iterator_operations(
        [](auto aIterator, const auto aDimensionIndex, const auto& aBaseVector,
           const test_utilities::TestContext& aTestContext)
        {
            auto tPreviousIterator = aIterator++;
            EXPECT_EQ(*tPreviousIterator, aBaseVector.at(aDimensionIndex)) << aTestContext;
            EXPECT_EQ(*aIterator, aBaseVector.at(aDimensionIndex + kDimensions)) << aTestContext;

            tPreviousIterator = aIterator++;
            EXPECT_EQ(*tPreviousIterator, aBaseVector.at(aDimensionIndex + kDimensions)) << aTestContext;
            EXPECT_EQ(*aIterator, aBaseVector.at(aDimensionIndex + 2 * kDimensions)) << aTestContext;
        },
        TEST_CONTEXT("Post-increment"));
}

TEST(SingleDimensionMultiVectorView, IteratorPreDecrement)
{
    check_iterator_operations(
        [](auto aIterator, const auto aDimensionIndex, const auto& aBaseVector,
           const test_utilities::TestContext& aTestContext)
        {
            ++aIterator;
            --aIterator;
            EXPECT_EQ(*aIterator, aBaseVector.at(aDimensionIndex)) << aTestContext;
        },
        TEST_CONTEXT("Pre-decrement"));
}

TEST(SingleDimensionMultiVectorView, IteratorPostDecrement)
{
    check_iterator_operations(
        [](auto aIterator, const auto aDimensionIndex, const auto& aBaseVector,
           const test_utilities::TestContext& aTestContext)
        {
            ++aIterator;
            auto tPreviousIterator = aIterator--;
            EXPECT_EQ(*tPreviousIterator, aBaseVector.at(aDimensionIndex + kDimensions)) << aTestContext;
            EXPECT_EQ(*aIterator, aBaseVector.at(aDimensionIndex)) << aTestContext;
        },
        TEST_CONTEXT("Post-decrement"));
}

TEST(SingleDimensionMultiVectorView, IteratorIntegerPlusOperator)
{
    check_iterator_operations(
        [](const auto& aIterator, const auto aDimensionIndex, const auto& aBaseVector,
           const test_utilities::TestContext& aTestContext)
        {
            constexpr auto tIncrement = 2;
            const auto tIncrementedIterator1 = aIterator + tIncrement;
            EXPECT_EQ(*tIncrementedIterator1, aBaseVector.at(aDimensionIndex + tIncrement * kDimensions))
                << aTestContext;

            const auto tIncrementedIterator2 = tIncrement + aIterator;
            EXPECT_EQ(*tIncrementedIterator2, *tIncrementedIterator1) << aTestContext;
        },
        TEST_CONTEXT("Integer plus operator"));
}

TEST(SingleDimensionMultiVectorView, IteratorIntegerMinusOperator)
{
    check_iterator_operations(
        [](const auto& aIterator, const auto aDimensionIndex, const auto& aBaseVector,
           const test_utilities::TestContext& aTestContext)
        {
            constexpr auto tIncrement = 3;
            const auto tIncrementedIterator = aIterator + tIncrement;
            constexpr auto tDecrement = 2;
            const auto tDecrementedIterator = tIncrementedIterator - tDecrement;
            EXPECT_EQ(*tDecrementedIterator, aBaseVector.at(aDimensionIndex + (tIncrement - tDecrement) * kDimensions))
                << aTestContext;
        },
        TEST_CONTEXT("Integer minus operator"));
}

TEST(SingleDimensionMultiVectorView, IteratorMinusOperator)
{
    check_iterator_operations(
        [](const auto& aIterator, const auto, const auto&, const test_utilities::TestContext& aTestContext)
        {
            constexpr auto tIncrement = 3;
            auto tIncrementedIterator = aIterator + tIncrement;

            EXPECT_EQ(tIncrementedIterator - aIterator, tIncrement) << aTestContext;
            EXPECT_EQ(aIterator - tIncrementedIterator, -tIncrement) << aTestContext;

            --tIncrementedIterator;
            EXPECT_EQ(tIncrementedIterator - aIterator, tIncrement - 1) << aTestContext;
            EXPECT_EQ(aIterator - tIncrementedIterator, -tIncrement + 1) << aTestContext;
        },
        TEST_CONTEXT("Minus operator"));
}

TEST(SingleDimensionMultiVectorView, IteratorPlusEqualOperator)
{
    check_iterator_operations(
        [](auto aIterator, const auto aDimensionIndex, const auto& aBaseVector,
           const test_utilities::TestContext& aTestContext)
        {
            constexpr auto tIncrement = 2;
            aIterator += tIncrement;

            EXPECT_EQ(*aIterator, aBaseVector.at(aDimensionIndex + tIncrement * kDimensions)) << aTestContext;

            aIterator += tIncrement;
            EXPECT_EQ(*aIterator,
                      aBaseVector.at(aDimensionIndex + static_cast<std::size_t>(2) * tIncrement * kDimensions))
                << aTestContext;
        },
        TEST_CONTEXT("Plus equal operator"));
}

TEST(SingleDimensionMultiVectorView, IteratorMinusEqualOperator)
{
    check_iterator_operations(
        [](auto aIterator, const auto aDimensionIndex, const auto& aBaseVector,
           const test_utilities::TestContext& aTestContext)
        {
            constexpr auto tDecrement = 2;
            aIterator += kLength - 1;
            aIterator -= tDecrement;
            EXPECT_EQ(*aIterator, aBaseVector.at(aDimensionIndex + (kLength - 1 - tDecrement) * kDimensions))
                << aTestContext;

            aIterator -= tDecrement;
            EXPECT_EQ(*aIterator,
                      aBaseVector.at(aDimensionIndex +
                                     (kLength - 1 - static_cast<std::size_t>(2) * tDecrement) * kDimensions))
                << aTestContext;
        },
        TEST_CONTEXT("Minus equal operator"));
}

TEST(SingleDimensionMultiVectorView, IteratorConstAccessOperator)
{
    check_iterator_operations(
        [](const auto& aIterator, const auto aDimensionIndex, const auto& aBaseVector,
           const test_utilities::TestContext& aTestContext)
        {
            for (const auto tVectorIndex : std::views::iota(0U, kLength))
            {
                EXPECT_EQ(aIterator[tVectorIndex], aBaseVector.at(aDimensionIndex + tVectorIndex * kDimensions))
                    << aTestContext;
            }
        },
        TEST_CONTEXT("Const access operator"));
}

TEST(SingleDimensionMultiVectorView, IteratorMutableAccessOperator)
{
    check_iterator_operations(
        [](auto aIterator, const auto aDimensionIndex, const auto& aBaseVector,
           const test_utilities::TestContext& aTestContext)
        {
            for (const auto tVectorIndex : std::views::iota(0U, kLength))
            {
                EXPECT_EQ(aIterator[tVectorIndex], aBaseVector.at(aDimensionIndex + tVectorIndex * kDimensions))
                    << aTestContext;

                const auto tNewValue = tVectorIndex * 42U;
                aIterator[tVectorIndex] = tNewValue;

                EXPECT_EQ(aIterator[tVectorIndex], tNewValue) << aTestContext;
            }
        },
        TEST_CONTEXT("Mutable access operator"));
}

TEST(SingleDimensionMultiVectorView, IteratorEquality)
{
    check_iterator_operations(
        [](auto aIterator, const auto, const auto&, const test_utilities::TestContext& aTestContext)
        {
            EXPECT_TRUE(aIterator == aIterator) << aTestContext;
            EXPECT_FALSE(aIterator != aIterator) << aTestContext;

            const auto tIncrementedIterator = aIterator + 1;
            EXPECT_FALSE(aIterator == tIncrementedIterator) << aTestContext;
            EXPECT_TRUE(aIterator != tIncrementedIterator) << aTestContext;
        },
        TEST_CONTEXT("Iterator equality"));
}

TEST(SingleDimensionMultiVectorView, IteratorInequalities)
{
    check_iterator_operations(
        [](auto aIterator, const auto, const auto&, const test_utilities::TestContext& aTestContext)
        {
            EXPECT_FALSE(aIterator < aIterator) << aTestContext;
            EXPECT_TRUE(aIterator <= aIterator) << aTestContext;
            EXPECT_FALSE(aIterator > aIterator) << aTestContext;
            EXPECT_TRUE(aIterator >= aIterator) << aTestContext;

            const auto tIncrementedIterator = aIterator + 1;
            EXPECT_TRUE(aIterator < tIncrementedIterator) << aTestContext;
            EXPECT_TRUE(aIterator <= tIncrementedIterator) << aTestContext;
            EXPECT_FALSE(aIterator > tIncrementedIterator) << aTestContext;
            EXPECT_FALSE(aIterator >= tIncrementedIterator) << aTestContext;

            EXPECT_FALSE(tIncrementedIterator < aIterator) << aTestContext;
            EXPECT_FALSE(tIncrementedIterator <= aIterator) << aTestContext;
            EXPECT_TRUE(tIncrementedIterator > aIterator) << aTestContext;
            EXPECT_TRUE(tIncrementedIterator >= aIterator) << aTestContext;
        },
        TEST_CONTEXT("Iterator inequalities"));
}

TEST(SingleDimensionMultiVectorView, IteratorSentinelEquality)
{
    check_iterator_operations(
        [](auto aIterator, const auto, const auto&, const test_utilities::TestContext& aTestContext)
        {
            const auto tEndSentinel = typename SingleDimensionMultiVectorView<std::vector<unsigned int>>::EndSentinel{};
            EXPECT_TRUE(aIterator != tEndSentinel) << aTestContext;
            EXPECT_FALSE(aIterator == tEndSentinel) << aTestContext;
            EXPECT_TRUE(tEndSentinel != aIterator) << aTestContext;
            EXPECT_FALSE(tEndSentinel == aIterator) << aTestContext;

            aIterator += kLength;
            EXPECT_TRUE(aIterator == tEndSentinel) << aTestContext;
            EXPECT_FALSE(aIterator != tEndSentinel) << aTestContext;
            EXPECT_TRUE(tEndSentinel == aIterator) << aTestContext;
            EXPECT_FALSE(tEndSentinel != aIterator) << aTestContext;
        },
        TEST_CONTEXT("Sentinel equality"));
}

TEST(SingleDimensionMultiVectorView, IteratorSentinelDifference)
{
    check_iterator_operations(
        [](auto aIterator, const auto, const auto&, const test_utilities::TestContext& aTestContext)
        {
            const auto tEndSentinel = typename SingleDimensionMultiVectorView<std::vector<unsigned int>>::EndSentinel{};
            const auto tLengthAsInt = static_cast<int>(kLength);
            EXPECT_EQ(aIterator - tEndSentinel, -tLengthAsInt) << aTestContext;
            EXPECT_EQ(tEndSentinel - aIterator, tLengthAsInt) << aTestContext;

            ++aIterator;
            EXPECT_EQ(aIterator - tEndSentinel, -tLengthAsInt + 1) << aTestContext;
            EXPECT_EQ(tEndSentinel - aIterator, tLengthAsInt - 1) << aTestContext;
        },
        TEST_CONTEXT("Iterator-sentinel difference"));
}

TEST(SingleDimensionMultiVectorView, BeginEndIterators)
{
    auto tVector = test_vector();
    auto tMultiVectorView = MultiVectorView(tVector, kDimensions);

    for (const auto tDimensionIndex : std::views::iota(0U, kDimensions))
    {
        const auto tBeginIterator = SingleDimensionMultiVectorView{tMultiVectorView, tDimensionIndex}.begin();
        EXPECT_EQ(*tBeginIterator, tVector.at(tDimensionIndex));

        const auto tEndIterator = SingleDimensionMultiVectorView{tMultiVectorView, tDimensionIndex}.end();
        EXPECT_NE(tBeginIterator, tEndIterator);
    }
}

TEST(SingleDimensionMultiVectorView, Copy)
{
    constexpr auto tDimension = std::size_t{2};
    constexpr auto tLength = std::size_t{5};

    auto tEntryGenerator = std::views::iota(0U, tLength * tDimension) | std::views::common;
    const auto tVector = std::vector(tEntryGenerator.begin(), tEntryGenerator.end());

    const auto tTest = [&](const auto tIndex, const test_utilities::TestContext& aTestContext)
    {
        auto tCoordinateRange = SingleDimensionMultiVectorView{MultiVectorView{tVector, tDimension}, tIndex};

        const auto tExpectedEntries =
            std::views::iota(0U) |
            std::views::transform([tIndex](const auto tEntry) { return tDimension * tEntry + tIndex; }) |
            std::views::take(tLength) | std::views::common;
        const auto tExpected = std::vector(tExpectedEntries.begin(), tExpectedEntries.end());

        const auto tResult = copy_random_access_range(tCoordinateRange);
        EXPECT_EQ(tExpected, tResult) << aTestContext;
    };

    constexpr auto tXIndex = 0U;
    tTest(tXIndex, TEST_CONTEXT("X index"));

    constexpr auto tYIndex = 1U;
    tTest(tYIndex, TEST_CONTEXT("Y index"));
}

}  // namespace plato::utilities::unittest
