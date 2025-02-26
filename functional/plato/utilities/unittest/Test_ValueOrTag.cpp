#include <gtest/gtest.h>

#include "plato/utilities/ValueOrTag.hpp"
#include "plato/utilities/unittest/CopyCounter.hpp"

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

using MaybeCopyCounter = ValueOrTag<CopyCounter, Nothing>;

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
    const auto tResult = tValue.valueOr("Hero");
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
    const auto tResult = tValue.valueOrInvoke([&tHero]() { return tHero; });
    EXPECT_EQ(tResult, tHero);
}

TEST(ValueOrTag, Copy)
{
    {
        const auto tValue = MaybeCopyCounter{CopyCounter{}};
        const auto tResult = tValue.valueOr(CopyCounter{});
        constexpr auto tExpectedNumberOfCopies = 1U;
        EXPECT_EQ(tResult.mCopies, tExpectedNumberOfCopies);

        const auto tInvokeResult = tValue.valueOrInvoke([]() { return CopyCounter{}; });
        EXPECT_EQ(tInvokeResult.mCopies, tExpectedNumberOfCopies);
    }
    {
        const auto tValue = MaybeCopyCounter{Nothing{}};
        const auto tResult = tValue.valueOr(CopyCounter{});
        constexpr auto tExpectedNumberOfCopies = 0U;
        EXPECT_EQ(tResult.mCopies, tExpectedNumberOfCopies);

        const auto tInvokeResult = tValue.valueOrInvoke([]() { return CopyCounter{}; });
        EXPECT_EQ(tInvokeResult.mCopies, tExpectedNumberOfCopies);
    }
}

TEST(ValueOrTag, Move)
{
    auto tValue = MaybeCopyCounter{CopyCounter{}};
    const auto tResult = std::move(tValue).valueOr(CopyCounter{});
    constexpr auto tExpectedNumberOfCopies = 0U;
    EXPECT_EQ(tResult.mCopies, tExpectedNumberOfCopies);
    constexpr auto tExpectedNumberOfMoves = 2U;
    EXPECT_EQ(tResult.mMoves, tExpectedNumberOfMoves);

    const auto tInvokeResult = std::move(tValue).valueOrInvoke([]() { return CopyCounter{}; });
    EXPECT_EQ(tInvokeResult.mCopies, tExpectedNumberOfCopies);
    EXPECT_EQ(tInvokeResult.mMoves, tExpectedNumberOfMoves);
}

}  // namespace plato::utilities::unittest
