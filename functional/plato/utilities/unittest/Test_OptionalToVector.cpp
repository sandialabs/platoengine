#include <gtest/gtest.h>

#include "plato/test_utilities/CopyCounter.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/utilities/OptionalToVector.hpp"

namespace plato::utilities::unittest
{

TEST(OptionalToVector, EmptyOptional)
{
    const auto tEmptyOptional = std::optional<double>{};
    const auto tEmptyVector = optional_to_vector(tEmptyOptional);
    EXPECT_TRUE(tEmptyVector.empty());
}

TEST(OptionalToVector, DoubleOptionalWithValue)
{
    const auto tOptional = std::optional<double>{42.0};
    const auto tVector = optional_to_vector(tOptional);
    ASSERT_EQ(tVector.size(), 1u);
    EXPECT_EQ(tOptional.value(), tVector.front());
}

TEST(OptionalToVector, CopiesForLValue)
{
    const auto tTestFunction = [](const auto& aVector, const test_utilities::TestContext& aTestContext)
    {
        ASSERT_EQ(aVector.size(), 1U) << aTestContext;
        EXPECT_EQ(aVector.front().mCopies, 1) << aTestContext;
        EXPECT_EQ(aVector.front().mMoves, 0) << aTestContext;
    };
    {
        const auto tOptional = std::optional<test_utilities::CopyCounter>{std::in_place_t{}};
        const auto tVector = optional_to_vector(tOptional);
        tTestFunction(tVector, TEST_CONTEXT("Const l-value"));
    }
    {
        auto tOptional = std::optional<test_utilities::CopyCounter>{std::in_place_t{}};
        const auto tVector = optional_to_vector(tOptional);
        tTestFunction(tVector, TEST_CONTEXT("Non-const l-value"));
    }
}

TEST(OptionalToVector, MovesForPRValue)
{
    const auto tVector = optional_to_vector(std::optional<test_utilities::CopyCounter>{std::in_place_t{}});
    ASSERT_EQ(tVector.size(), 1u);
    EXPECT_EQ(tVector.front().mCopies, 0);
    EXPECT_EQ(tVector.front().mMoves, 1);
}

TEST(OptionalToVector, MovesForXValue)
{
    auto tOptional = std::optional<test_utilities::CopyCounter>{std::in_place_t{}};
    auto tVector = optional_to_vector(std::move(tOptional));
    ASSERT_EQ(tVector.size(), 1u);
    EXPECT_EQ(tVector.front().mCopies, 0);
    EXPECT_EQ(tVector.front().mMoves, 1);
}

}  // namespace plato::utilities::unittest
