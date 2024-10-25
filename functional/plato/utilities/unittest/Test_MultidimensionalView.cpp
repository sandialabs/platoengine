#include <gtest/gtest.h>

#include <numeric>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/utilities/IndexRange.hpp"
#include "plato/utilities/MultidimensionalView.hpp"

namespace plato::utilities::unittest
{
TEST(VectorView, Values)
{
    constexpr auto tDimension = std::size_t{2};
    constexpr auto tLength = std::size_t{5};
    const auto tVector = []()
    {
        auto tTempVector = std::vector<double>(tDimension * tLength);
        std::iota(tTempVector.begin(), tTempVector.end(), 0.0);
        return tTempVector;
    }();

    const auto tTest = [&tVector, tLength](auto& tMultidimensionalView, const test_utilities::TestContext& aTestContext)
    {
        auto tIndex = std::size_t{0};
        ASSERT_EQ(tMultidimensionalView.size(), tLength) << aTestContext;
        // Check in expected order
        for (const auto tLengthIndex : IndexRange{tLength})
        {
            for (const auto tDimensionIndex : IndexRange{tDimension})
            {
                EXPECT_EQ(tMultidimensionalView(tLengthIndex, tDimensionIndex), tVector[tIndex]) << aTestContext;
                ++tIndex;
            }
        }
    };

    auto tNonConstView = MultidimensionalView<tDimension, const std::vector<double>>{tVector};
    tTest(tNonConstView, TEST_CONTEXT("Non-const view"));

    const auto tConstView = MultidimensionalView<tDimension, const std::vector<double>>{tVector};
    tTest(tConstView, TEST_CONTEXT("Const view"));
}
}  // namespace plato::utilities::unittest
