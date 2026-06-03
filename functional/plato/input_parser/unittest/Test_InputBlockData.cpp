#include <gtest/gtest.h>

#include <boost/none.hpp>
#include <boost/optional/optional.hpp>
#include <utility>

#include "plato/input_parser/InputBlockData.hpp"
#include "plato/test_utilities/CopyCounter.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::input_parser::unittest
{
namespace
{
struct Active
{
    // Uses `active` instead of `mActive` to match input blocks
    boost::optional<bool> active = boost::none;  // NOLINT(readability-identifier-naming)
};
}  // namespace

TEST(InputBlockWrapper, SetAndGet)
{
    InputBlockWrapper tInput{};
    // r-value
    {
        constexpr auto tArbitraryValue = 88;
        tInput.set(tArbitraryValue);
        ASSERT_NO_THROW([[maybe_unused]] const auto tObject = tInput.get<int>());
        EXPECT_EQ(tInput.get<int>(), tArbitraryValue);
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

TEST(InputBlockWrapper, HoldsExpectedType)
{
    constexpr auto tArbitraryValue = 86;
    InputBlockWrapper tInput{};
    tInput.set(tArbitraryValue);
    EXPECT_FALSE(tInput.holdsExpectedType<double>());
    EXPECT_FALSE(tInput.holdsExpectedType<unsigned int>());
    EXPECT_FALSE(tInput.holdsExpectedType<char>());
    EXPECT_TRUE(tInput.holdsExpectedType<int>());
}

TEST(InputBlockWrapper, HasValue)
{
    InputBlockWrapper tInput{};
    EXPECT_FALSE(tInput.hasValue());
    constexpr auto tArbitraryValue = 38;
    tInput.set(tArbitraryValue);
    EXPECT_TRUE(tInput.hasValue());
}

TEST(InputBlockWrapper, MutatingGet)
{
    constexpr auto tArbitraryInitialValue = 101;
    auto tInput = InputBlockWrapper{tArbitraryInitialValue};
    ASSERT_TRUE(tInput.holdsExpectedType<int>());
    constexpr auto tNewValue = 42;
    tInput.get<int>() = tNewValue;
    EXPECT_EQ(tInput.get<int>(), tNewValue);
}

TEST(InputBlockWrapper, Ctors)
{
    constexpr auto tValue = 42;
    auto tInput = InputBlockWrapper{tValue};
    const auto tCheckCrossReference =
        [tValue](const InputBlockWrapper& aInput, const test_utilities::TestContext& aTestContext)
    {
        ASSERT_TRUE(aInput.hasValue()) << aTestContext;
        ASSERT_TRUE(aInput.holdsExpectedType<int>()) << aTestContext;
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

        const auto tInputWithCounter = InputBlockWrapper{test_utilities::CopyCounter{}};
        const auto tInputWithCounter2 = tInputWithCounter;  // NOLINT
        EXPECT_EQ(tInputWithCounter2.get<test_utilities::CopyCounter>().mCopies, 1U);
        EXPECT_EQ(tInputWithCounter2.get<test_utilities::CopyCounter>().mMoves, 1U);
    }
    // Move ctor
    {
        const auto tInput2 = std::move(tInput);
        tCheckCrossReference(tInput2, TEST_CONTEXT("Move constructor"));

        auto tInputWithCounter = InputBlockWrapper{test_utilities::CopyCounter{}};
        const auto tInputWithCounter2 = std::move(tInputWithCounter);
        EXPECT_EQ(tInputWithCounter2.get<test_utilities::CopyCounter>().mCopies, 0U);
        EXPECT_EQ(tInputWithCounter2.get<test_utilities::CopyCounter>().mMoves, 2U);
    }
}

TEST(InputBlockWrapper, Active)
{
    // Default
    {
        const auto tInput = InputBlockWrapper{Active{}};
        EXPECT_TRUE(tInput.active());
    }
    // True
    {
        const auto tInput = InputBlockWrapper{Active{true}};
        EXPECT_TRUE(tInput.active());
    }
    // False
    {
        const auto tInput = InputBlockWrapper{Active{false}};
        EXPECT_FALSE(tInput.active());
    }
    // No active field
    {
        const auto tInput = InputBlockWrapper{42};
        EXPECT_TRUE(tInput.active());
    }
}

}  // namespace plato::input_parser::unittest
