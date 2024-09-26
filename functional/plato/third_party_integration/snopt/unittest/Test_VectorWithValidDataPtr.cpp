#include <gtest/gtest.h>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/snopt/VectorWithValidDataPtr.hpp"

namespace plato::third_party_integration::snopt::unittest
{
TEST(VectorWithValidDataPtr, Uninitialized)
{
    // Const
    {
        const auto tVector = VectorWithValidDataPtr<double>{};
        EXPECT_EQ(tVector.size(), 0U);
        EXPECT_NE(tVector.data(), nullptr);
    }
    // Non-const
    {
        auto tVector = VectorWithValidDataPtr<double>{};
        EXPECT_EQ(tVector.size(), 0U);
        EXPECT_NE(tVector.data(), nullptr);
    }
}

TEST(VectorWithValidDataPtr, ConstructionWithSize)
{
    const auto tCheckSizeAndDataPtr = [](const std::size_t aSize, const test_utilities::TestContext& aTestContext)
    {
        const auto tVector = VectorWithValidDataPtr<int>(aSize);
        EXPECT_EQ(tVector.size(), aSize) << aTestContext;
        EXPECT_NE(tVector.data(), nullptr) << aTestContext;
    };
    tCheckSizeAndDataPtr(0U, TEST_CONTEXT("Construction with zero size"));
    tCheckSizeAndDataPtr(42U, TEST_CONTEXT("Construction with non-zero size"));
}

TEST(VectorWithValidDataPtr, PushBack)
{
    auto tVector = VectorWithValidDataPtr<int>{};
    constexpr auto tEntry = int{42};
    tVector.pushBack(tEntry);

    EXPECT_EQ(tVector.size(), 1U);
    ASSERT_NE(tVector.data(), nullptr);
    EXPECT_EQ(tVector.data()[0], tEntry);
}

TEST(VectorWithValidDataPtr, EntryAccessor)
{
    auto tVector = VectorWithValidDataPtr<int>{};
    constexpr auto tEntry = int{42};
    tVector.pushBack(tEntry);
    tVector.pushBack(2 * tEntry);

    EXPECT_EQ(tVector.size(), 2U);
    ASSERT_NE(tVector.data(), nullptr);
    EXPECT_EQ(tVector[0], tEntry);
    EXPECT_EQ(tVector[1], 2 * tEntry);

    // Modify and re-check
    tVector[0] = 3 * tEntry;
    EXPECT_EQ(tVector[0], 3 * tEntry);
    EXPECT_EQ(tVector[1], 2 * tEntry);
}

}  // namespace plato::third_party_integration::snopt::unittest
