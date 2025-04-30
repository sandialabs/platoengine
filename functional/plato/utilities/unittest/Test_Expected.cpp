#include <gtest/gtest.h>

#include "plato/test_utilities/CopyCounter.hpp"
#include "plato/utilities/Expected.hpp"

namespace plato::utilities::unittest
{
TEST(Expected, IsUnexpected)
{
    EXPECT_FALSE(kIsUnexpected<int>);
    EXPECT_TRUE(kIsUnexpected<Unexpected<int>>);
}

TEST(Expected, HasValueBoolConversion)
{
    {
        const auto tExpected = Expected<int, std::string>{42};
        EXPECT_TRUE(tExpected.hasValue());
        EXPECT_FALSE(tExpected.hasError());
        EXPECT_TRUE(tExpected);
    }
    {
        const auto tExpected = Expected<int, std::string>{Unexpected<std::string>{"Error!"}};
        EXPECT_FALSE(tExpected.hasValue());
        EXPECT_TRUE(tExpected.hasError());
        EXPECT_FALSE(tExpected);
    }
}

TEST(Expected, ValueConstRef)
{
    {
        constexpr auto tValue = 42;
        const auto tExpected = Expected<int, std::string>{tValue};
        EXPECT_EQ(tExpected.value(), tValue);
    }
    {
        const auto tExpected = Expected<int, std::string>{Unexpected<std::string>{"Error!"}};
        EXPECT_THROW([[maybe_unused]] const auto tResult = tExpected.value(), Exception);
    }
}

TEST(Expected, ValueRValueOverload)
{
    {
        auto tExpected = Expected<test_utilities::CopyCounter, std::string>{test_utilities::CopyCounter{}};
        const auto tMovedCopyCounter = std::move(tExpected).value();
        EXPECT_EQ(tMovedCopyCounter.mMoves, 2U);
        EXPECT_EQ(tMovedCopyCounter.mCopies, 0U);
    }
    {
        auto tExpected = Expected<int, std::string>{Unexpected<std::string>{"Error!"}};
        EXPECT_THROW([[maybe_unused]] const auto tResult = std::move(tExpected).value(), Exception);
    }
}

TEST(Expected, ValueNonconstRef)
{
    {
        constexpr auto tValue = 42;
        auto tExpected = Expected<int, std::string>{-1};
        tExpected.value() = 42;
        EXPECT_EQ(tExpected.value(), tValue);
    }
    {
        auto tExpected = Expected<int, std::string>{Unexpected<std::string>{"Error!"}};
        EXPECT_THROW([[maybe_unused]] const auto tResult = tExpected.value(), Exception);
    }
}

TEST(Expected, BadAccessException)
{
    const auto tErrorMessage = std::string{"Error!"};
    try
    {
        const auto tExpected = Expected<int, std::string>{Unexpected<std::string>{tErrorMessage}};
        [[maybe_unused]] const auto& tResult = tExpected.value();
    }
    catch (const Exception& aError)
    {
        EXPECT_EQ(aError.what(), tErrorMessage);
    }
}

TEST(Expected, Error)
{
    const auto tErrorMessage = std::string{"Error!"};
    const auto tExpected = Expected<int, std::string>{Unexpected<std::string>{tErrorMessage}};
    EXPECT_EQ(tExpected.error(), tErrorMessage);
}

TEST(Expected, AssignFromUnexpected)
{
    const auto tErrorMessage = "Error!";
    auto tExpected = Expected<int, std::string>{42};
    tExpected = unexpected(tErrorMessage);
    EXPECT_FALSE(tExpected.hasValue());
    EXPECT_EQ(tExpected.error(), tErrorMessage);
}

}  // namespace plato::utilities::unittest
