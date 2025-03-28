#include <gtest/gtest.h>

#include "plato/input_parser/CrossReference.hpp"
#include "plato/test_utilities/CopyCounter.hpp"
#include "plato/test_utilities/TestContext.hpp"

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

TEST(CrossReferencedInput, MutatingGet)
{
    auto tInput = CrossReferencedInput{101};
    ASSERT_TRUE(tInput.holds_expected_type<int>());
    constexpr auto tNewValue = 42;
    tInput.get<int>() = tNewValue;
    EXPECT_EQ(tInput.get<int>(), tNewValue);
}

TEST(CrossReferencedInput, Ctors)
{
    constexpr auto tValue = int{42};
    auto tInput = CrossReferencedInput{tValue};
    const auto tCheckCrossReference =
        [tValue](const CrossReferencedInput& aInput, const test_utilities::TestContext& aTestContext)
    {
        ASSERT_TRUE(aInput.has_value()) << aTestContext;
        ASSERT_TRUE(aInput.holds_expected_type<int>()) << aTestContext;
        EXPECT_EQ(aInput.get<int>(), tValue) << aTestContext;
    };
    // Converting ctor
    {
        tCheckCrossReference(tInput, TEST_CONTEXT("Converting constructor"));
    }
    // Copy ctor
    {
        const auto tInputCopy = tInput;  // NOLINT
        tCheckCrossReference(tInputCopy, TEST_CONTEXT("Copy constructor"));

        const auto tInputWithCounter = CrossReferencedInput{test_utilities::CopyCounter{}};
        const auto tInputWithCounter2 = tInputWithCounter;  // NOLINT
        EXPECT_EQ(tInputWithCounter2.get<test_utilities::CopyCounter>().mCopies, 1U);
        EXPECT_EQ(tInputWithCounter2.get<test_utilities::CopyCounter>().mMoves, 1U);
    }
    // Move ctor
    {
        const auto tInput2 = std::move(tInput);
        tCheckCrossReference(tInput2, TEST_CONTEXT("Move constructor"));

        auto tInputWithCounter = CrossReferencedInput{test_utilities::CopyCounter{}};
        const auto tInputWithCounter2 = std::move(tInputWithCounter);
        EXPECT_EQ(tInputWithCounter2.get<test_utilities::CopyCounter>().mCopies, 0U);
        EXPECT_EQ(tInputWithCounter2.get<test_utilities::CopyCounter>().mMoves, 2U);
    }
}

}  // namespace plato::input_parser::unittest
