#include <gtest/gtest.h>

#include "plato/input_parser/CrossReference.hpp"

namespace plato::input_parser::unittest
{
TEST(CrossReferencedInput, SetAndGet)
{
    CrossReferencedInput tInput{};
    // r-value
    {
        tInput.set(88);
        ASSERT_NO_THROW([[maybe_unused]] const auto tObject = tInput.get<int>());
        EXPECT_EQ(tInput.get<int>(), 88);
    }
    // l-value
    {
        constexpr unsigned int tInputBlock = 91;
        tInput.set(tInputBlock);
        ASSERT_NO_THROW([[maybe_unused]] const auto tObject = tInput.get<unsigned int>());
        EXPECT_EQ(tInput.get<unsigned int>(), 91);
    }
    // l-value temporary
    {
        {
            constexpr int tInputBlock = 71;
            tInput.set(tInputBlock);
        }
        ASSERT_NO_THROW([[maybe_unused]] const auto tObject = tInput.get<int>());
        EXPECT_EQ(tInput.get<int>(), 71);
    }
}

TEST(CrossReferencedInput, GetThrowsWithWrongType)
{
    CrossReferencedInput tInput{};
    tInput.set(77);
    ASSERT_THROW([[maybe_unused]] const auto tObject = tInput.get<double>(), std::bad_any_cast);
    EXPECT_NO_THROW([[maybe_unused]] const auto tObject = tInput.get<int>());
}

TEST(CrossReferencedInput, HoldsExpectedType)
{
    CrossReferencedInput tInput{};
    tInput.set(86);
    EXPECT_FALSE(tInput.holds_expected_type<double>());
    EXPECT_FALSE(tInput.holds_expected_type<unsigned int>());
    EXPECT_FALSE(tInput.holds_expected_type<char>());
    EXPECT_TRUE(tInput.holds_expected_type<int>());
}

TEST(CrossReferencedInput, HasValue)
{
    CrossReferencedInput tInput{};
    EXPECT_FALSE(tInput.has_value());
    tInput.set(38);
    EXPECT_TRUE(tInput.has_value());
}
}  // namespace plato::input_parser::unittest