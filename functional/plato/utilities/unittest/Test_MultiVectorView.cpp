#include <gtest/gtest.h>

#include <numeric>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/utilities/IndexRange.hpp"
#include "plato/utilities/MultiVectorView.hpp"

namespace plato::utilities::unittest
{
namespace
{
struct TestVector
{
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};
}  // namespace

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

    auto tNonConstView = MultiVectorView{tVector, tDimension};
    tTest(tNonConstView, TEST_CONTEXT("Non-const view"));

    const auto tConstView = MultiVectorView{tVector, tDimension};
    tTest(tConstView, TEST_CONTEXT("Const view"));
}

TEST(MultiVectorView, Assignment)
{
    constexpr auto tDimension = std::size_t{2};
    constexpr auto tLength = std::size_t{5};
    auto tVector = std::vector<double>(tDimension * tLength, 0.0);
    auto tMultiVectorView = MultiVectorView{tVector, tDimension};

    constexpr auto tNewValue = 42.0;
    EXPECT_NE(tMultiVectorView(VectorIndex{0}, ComponentIndex{0}), tNewValue);
    tMultiVectorView(VectorIndex{0}, ComponentIndex{0}) = tNewValue;
    EXPECT_EQ(tMultiVectorView(VectorIndex{0}, ComponentIndex{0}), tNewValue);

    EXPECT_NE(tMultiVectorView(VectorIndex{1}, ComponentIndex{0}), tNewValue);
    tMultiVectorView(VectorIndex{1}, ComponentIndex{0}) = tNewValue;
    EXPECT_EQ(tMultiVectorView(VectorIndex{1}, ComponentIndex{0}), tNewValue);

    EXPECT_NE(tMultiVectorView(VectorIndex{0}, ComponentIndex{1}), tNewValue);
    tMultiVectorView(VectorIndex{0}, ComponentIndex{1}) = tNewValue;
    EXPECT_EQ(tMultiVectorView(VectorIndex{0}, ComponentIndex{1}), tNewValue);
}

TEST(MultiVectorView, Sizes)
{
    constexpr auto tDimension = std::size_t{3};
    constexpr auto tLength = std::size_t{2};
    const auto tVector = std::vector<double>(tDimension * tLength);

    const auto tMultiVectorView = utilities::make_multi_vector_view(tVector, tDimension);
    EXPECT_EQ(tMultiVectorView.numberOfVectors(), tLength);
    EXPECT_EQ(tMultiVectorView.size(), tLength * tDimension);
    EXPECT_EQ(tMultiVectorView.dimensions(), tDimension);
}

TEST(MultiVectorView, ShallowEquality)
{
    constexpr auto tDimension = std::size_t{3};
    constexpr auto tLength = std::size_t{2};
    const auto tVector1 = std::vector<double>(tDimension * tLength);

    const auto tMultiVectorView1 = utilities::make_multi_vector_view(tVector1, tDimension);

    EXPECT_TRUE(tMultiVectorView1.shallowEquality(tMultiVectorView1));

    // Constructed with same vector and size
    {
        const auto tMultiVectorView2 = utilities::make_multi_vector_view(tVector1, tDimension);
        EXPECT_TRUE(tMultiVectorView1.shallowEquality(tMultiVectorView2));
    }
    // Constructed with different vector, but same size
    {
        const auto tVector2 = std::vector<double>(tDimension * tLength);
        const auto tMultiVectorView2 = utilities::make_multi_vector_view(tVector2, tDimension);
        EXPECT_FALSE(tMultiVectorView1.shallowEquality(tMultiVectorView2));
    }
    // Constructed with same vector, but different dimensions
    {
        const auto tMultiVectorView2 = utilities::make_multi_vector_view(tVector1, tLength);
        EXPECT_FALSE(tMultiVectorView1.shallowEquality(tMultiVectorView2));
    }
}

TEST(MultiVectorView, VectorAssignment)
{
    constexpr auto tLength = std::size_t{2};

    // 2D view
    {
        constexpr auto tDimension = std::size_t{2};
        auto tVector = std::vector<double>(tDimension * tLength, 0.0);
        auto tMultiVectorView = MultiVectorView{tVector, tDimension};

        tMultiVectorView(VectorIndex{0}) = TestVector{1., 2.};
        EXPECT_EQ(tMultiVectorView(VectorIndex{0}, ComponentIndex{0}), 1.0);
        EXPECT_EQ(tMultiVectorView(VectorIndex{0}, ComponentIndex{1}), 2.0);

        tMultiVectorView(VectorIndex{1}) += TestVector{5., 6., 7.};
        EXPECT_EQ(tMultiVectorView(VectorIndex{1}, ComponentIndex{0}), 5.0);
        EXPECT_EQ(tMultiVectorView(VectorIndex{1}, ComponentIndex{1}), 6.0);

        tMultiVectorView(VectorIndex{1}) += TestVector{5., 6., 7.};
        EXPECT_EQ(tMultiVectorView(VectorIndex{1}, ComponentIndex{0}), 10.0);
        EXPECT_EQ(tMultiVectorView(VectorIndex{1}, ComponentIndex{1}), 12.0);
    }

    // 3D view
    {
        constexpr auto tDimension = std::size_t{3};
        auto tVector = std::vector<double>(tDimension * tLength, 0.0);
        auto tMultiVectorView = MultiVectorView{tVector, tDimension};

        tMultiVectorView(VectorIndex{0}) = TestVector{1., 2., 3.};
        EXPECT_EQ(tMultiVectorView(VectorIndex{0}, ComponentIndex{0}), 1.0);
        EXPECT_EQ(tMultiVectorView(VectorIndex{0}, ComponentIndex{1}), 2.0);
        EXPECT_EQ(tMultiVectorView(VectorIndex{0}, ComponentIndex{2}), 3.0);

        tMultiVectorView(VectorIndex{0}) = TestVector{21., 72., 34.};
        EXPECT_EQ(tMultiVectorView(VectorIndex{0}, ComponentIndex{0}), 21.0);
        EXPECT_EQ(tMultiVectorView(VectorIndex{0}, ComponentIndex{1}), 72.0);
        EXPECT_EQ(tMultiVectorView(VectorIndex{0}, ComponentIndex{2}), 34.0);
    }
}

TEST(MultiVectorView, VectorObjectFromVectorIndex)
{
    // 2D from a non-const vector
    {
        constexpr auto tDimension = std::size_t{2};
        auto tVector = std::vector<double>{1.0, 2.0, 3.0, 4.0};
        const auto tMultiVectorView = MultiVectorView{tVector, tDimension};
        const TestVector tVectorObject = tMultiVectorView(VectorIndex{0});
        EXPECT_EQ(tVectorObject.x, 1.0);
        EXPECT_EQ(tVectorObject.y, 2.0);
        EXPECT_EQ(tVectorObject.z, 0.0);
    }

    // 3D from a const vector
    {
        constexpr auto tDimension = std::size_t{3};
        const auto tVector = std::vector<double>{1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
        const auto tMultiVectorView = MultiVectorView{tVector, tDimension};
        const TestVector tVectorObject = tMultiVectorView(VectorIndex{1});
        EXPECT_EQ(tVectorObject.x, 4.0);
        EXPECT_EQ(tVectorObject.y, 5.0);
        EXPECT_EQ(tVectorObject.z, 6.0);
    }
}

}  // namespace plato::utilities::unittest
