#include <gtest/gtest.h>

#include <numeric>
#include <ranges>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/utilities/IndexRange.hpp"
#include "plato/utilities/MultiVectorView.hpp"

namespace plato::utilities::unittest
{
TEST(MultiVectorView, Values)
{
    constexpr auto tDimension = std::size_t{2};
    constexpr auto tLength = std::size_t{5};
    const auto tVector = []()
    {
        auto tTempVector = std::vector<double>(tDimension * tLength);
        std::iota(tTempVector.begin(), tTempVector.end(), 0.0);
        return tTempVector;
    }();

    const auto tTest = [&tVector, tLength](auto& tMultiVectorView, const test_utilities::TestContext& aTestContext)
    {
        auto tIndex = std::size_t{0};
        ASSERT_EQ(tMultiVectorView.size(), tLength * tDimension) << aTestContext;
        ASSERT_EQ(tMultiVectorView.numberOfVectors(), tLength) << aTestContext;
        // Check in expected order
        for (const auto tLengthIndex : IndexRange{tLength})
        {
            for (const auto tDimensionIndex : IndexRange{tDimension})
            {
                EXPECT_EQ(tMultiVectorView(VectorIndex{tLengthIndex}, ComponentIndex{tDimensionIndex}), tVector[tIndex])
                    << aTestContext;
                ++tIndex;
            }
        }
    };

    auto tNonConstView = MultiVectorView<const std::vector<double>>{tVector, tDimension};
    tTest(tNonConstView, TEST_CONTEXT("Non-const view"));

    const auto tConstView = MultiVectorView<const std::vector<double>>{tVector, tDimension};
    tTest(tConstView, TEST_CONTEXT("Const view"));
}

TEST(MultiVectorView, Sizes)
{
    constexpr auto tDimension = std::size_t{3};
    constexpr auto tLength = std::size_t{2};
    const auto tVector = std::vector<double>(tDimension * tLength);

    const auto tMultiVectorView = utilities::make_multi_vector_view(tVector, tDimension);
    EXPECT_EQ(tMultiVectorView.numberOfVectors(), tLength);
    EXPECT_EQ(tMultiVectorView.size(), tLength * tDimension);
}

}  // namespace plato::utilities::unittest
