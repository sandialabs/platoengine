#include <gtest/gtest.h>

#include <string>

#include "plato/utilities/BoostOptionalToStdOptional.hpp"

namespace plato::utilities::unittest
{
namespace
{
struct Wrapper
{
    int mChocolate = 0;
    double mPeanuts = 0.0;
    std::size_t mCaramel = 0;
};
}  // namespace

TEST(Utilities, BoostOptionalToStdOptional)
{
    {
        constexpr auto tGold = int{3};
        boost::optional<int> tBoostOptional = tGold;
        const auto tStdOptional = to_std_optional(tBoostOptional);

        ASSERT_TRUE(tStdOptional) << "int wrapped in optional";
        EXPECT_EQ(tStdOptional.value(), tBoostOptional.value()) << "int wrapped in optional";
    }
    {
        boost::optional<double> tBoostOptional = boost::none;
        const auto tStdOptional = to_std_optional(tBoostOptional);
        ASSERT_FALSE(tStdOptional) << "boost none double";
    }
    {
        boost::optional<std::string> tBoostOptional = std::string{"hello"};
        const auto tStdOptional = to_std_optional(tBoostOptional);
        ASSERT_TRUE(tStdOptional) << "string wrapped in optional";
        EXPECT_EQ(tStdOptional.value(), tBoostOptional.value()) << "string wrapped in optional";
    }
}

TEST(Utilities, ToUnwrappedOptionalEmpty)
{
    auto tBoostOptionalWrapper = boost::optional<Wrapper>{};
    const auto tStdOptionalPeanuts =
        to_unwrapped_optional(tBoostOptionalWrapper, [](const auto& aWrapper) { return aWrapper.mPeanuts; });
    EXPECT_FALSE(tStdOptionalPeanuts);
}

TEST(Utilities, ToUnwrappedOptional)
{
    constexpr auto tChocolate = int{10};
    constexpr auto tPeanuts = double{0.5};
    constexpr auto tCaramel = std::size_t{42};
    auto tBoostOptionalWrapper = boost::optional<Wrapper>{Wrapper{tChocolate, tPeanuts, tCaramel}};

    const auto tStdOptionalPeanuts =
        to_unwrapped_optional(tBoostOptionalWrapper, [](const auto& aWrapper) { return aWrapper.mPeanuts; });

    ASSERT_TRUE(tStdOptionalPeanuts);
    EXPECT_EQ(tStdOptionalPeanuts.value(), tPeanuts);
}

}  // namespace plato::utilities::unittest
