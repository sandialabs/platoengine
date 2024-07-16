#include <gtest/gtest.h>

#include "plato/utilities/OptionalToVector.hpp"
#include "plato/utilities/unittest/CopyCounter.hpp"

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

TEST(OptionalToVector, CopiesForConstLValue)
{
    const auto tOptional = std::optional<CopyCounter>{std::in_place_t{}};
    const auto tVector = optional_to_vector(tOptional);
    ASSERT_EQ(tVector.size(), 1u);
    EXPECT_EQ(tVector.front().mCopies, 1);
    EXPECT_EQ(tVector.front().mMoves, 0);
}

TEST(OptionalToVector, CopiesForNonconstLValue)
{
    auto tOptional = std::optional<CopyCounter>{std::in_place_t{}};
    const auto tVector = optional_to_vector(tOptional);
    ASSERT_EQ(tVector.size(), 1u);
    EXPECT_EQ(tVector.front().mCopies, 1);
    EXPECT_EQ(tVector.front().mMoves, 0);
}

TEST(OptionalToVector, MovesForPRValue)
{
    const auto tVector = optional_to_vector(std::optional<CopyCounter>{std::in_place_t{}});
    ASSERT_EQ(tVector.size(), 1u);
    EXPECT_EQ(tVector.front().mCopies, 0);
    EXPECT_EQ(tVector.front().mMoves, 1);
}

TEST(OptionalToVector, MovesForXValue)
{
    auto tOptional = std::optional<CopyCounter>{std::in_place_t{}};
    auto tVector = optional_to_vector(std::move(tOptional));
    ASSERT_EQ(tVector.size(), 1u);
    EXPECT_EQ(tVector.front().mCopies, 0);
    EXPECT_EQ(tVector.front().mMoves, 1);
}

}  // namespace plato::utilities::unittest
