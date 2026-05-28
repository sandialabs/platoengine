#include <gtest/gtest.h>

#include <string>
#include <utility>

#include "plato/test_utilities/CopyCounter.hpp"
#include "plato/utilities/ValueOrTag.hpp"

namespace plato::utilities::unittest
{
namespace
{
struct Hero
{
};

using MaybeMonster = ValueOrTag<std::string, Hero>;

struct Nothing
{
};

using MaybeCopyCounter = ValueOrTag<test_utilities::CopyCounter, Nothing>;

}  // namespace

TEST(ValueOrTag, HasValue)
{
    {
        const auto tValue = MaybeMonster{"demogorgon"};
        EXPECT_TRUE(tValue.hasValue());
    }
    {
        const auto tValue = MaybeMonster{Hero{}};
        EXPECT_FALSE(tValue.hasValue());
    }
}

TEST(ValueOrTag, HasTag)
{
    {
        const auto tValue = MaybeMonster{"demogorgon"};
        EXPECT_FALSE(tValue.has<Hero>());
    }
    {
        const auto tValue = MaybeMonster{Hero{}};
        EXPECT_TRUE(tValue.has<Hero>());
    }
}

TEST(ValueOrTag, ValueOrWithValue)
{
    const auto tLich = std::string{"lich"};
    const auto tValue = MaybeMonster{tLich};
    const auto tResult = tValue.valueOr(std::string{"Hero"});
    EXPECT_EQ(tResult, tLich);
}

TEST(ValueOrTag, ValueOrWithTag)
{
    const auto tValue = MaybeMonster{Hero{}};
    const auto tHero = std::string{"Hero"};
    const auto tResult = tValue.valueOr(tHero);
    EXPECT_EQ(tResult, tHero);
}

TEST(ValueOrTag, ValueOrInvokeWithValue)
{
    const auto tNightmare = std::string{"nightmare"};
    const auto tValue = MaybeMonster{tNightmare};
    static auto tInvokeCount = 0U;
    const auto tResult = tValue.valueOrInvoke(
        []()
        {
            ++tInvokeCount;
            return std::string{"Hero"};
        });
    EXPECT_EQ(tResult, tNightmare);
    EXPECT_EQ(tInvokeCount, 0U);
}

TEST(ValueOrTag, ValueOrInvokeWithTag)
{
    const auto tValue = MaybeMonster{Hero{}};
    const auto tHero = std::string{"Hero"};
    const auto tResult = tValue.valueOrInvoke([tHeroCopy = tHero]() { return tHeroCopy; });
    EXPECT_EQ(tResult, tHero);
}

TEST(ValueOrTag, Copy)
{
    {
        const auto tValue = MaybeCopyCounter{test_utilities::CopyCounter{}};
        const auto tResult = tValue.valueOr(test_utilities::CopyCounter{});
        constexpr auto tExpectedNumberOfCopies = 1U;
        EXPECT_EQ(tResult.mCopies, tExpectedNumberOfCopies);

        const auto tInvokeResult = tValue.valueOrInvoke([]() { return test_utilities::CopyCounter{}; });
        EXPECT_EQ(tInvokeResult.mCopies, tExpectedNumberOfCopies);
    }
    {
        const auto tValue = MaybeCopyCounter{Nothing{}};
        const auto tResult = tValue.valueOr(test_utilities::CopyCounter{});
        constexpr auto tExpectedNumberOfCopies = 0U;
        EXPECT_EQ(tResult.mCopies, tExpectedNumberOfCopies);

        const auto tInvokeResult = tValue.valueOrInvoke([]() { return test_utilities::CopyCounter{}; });
        EXPECT_EQ(tInvokeResult.mCopies, tExpectedNumberOfCopies);
    }
}

TEST(ValueOrTag, Move)
{
    constexpr auto tExpectedNumberOfCopies = 0U;
    constexpr auto tExpectedNumberOfMoves = 2U;
    {
        auto tValue = MaybeCopyCounter{test_utilities::CopyCounter{}};
        const auto tResult = std::move(tValue).valueOr(test_utilities::CopyCounter{});
        EXPECT_EQ(tResult.mCopies, tExpectedNumberOfCopies) << "valueOr";
        EXPECT_EQ(tResult.mMoves, tExpectedNumberOfMoves) << "valueOr";
    }
    {
        auto tValue = MaybeCopyCounter{test_utilities::CopyCounter{}};
        const auto tInvokeResult = std::move(tValue).valueOrInvoke([]() { return test_utilities::CopyCounter{}; });
        EXPECT_EQ(tInvokeResult.mCopies, tExpectedNumberOfCopies) << "valueOrInvoke";
        EXPECT_EQ(tInvokeResult.mMoves, tExpectedNumberOfMoves) << "valueOrInvoke";
    }
}

}  // namespace plato::utilities::unittest
