#include <gtest/gtest.h>

#include <string>

#include "plato/input_parser/FileList.hpp"
#include "plato/utilities/BoostOptionalToStdOptional.hpp"

namespace plato::utilities::unittest
{

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

TEST(Utilities, ToUnwrappedOptional)
{
    constexpr std::string_view tFileName = "test.log";
    boost::optional<input_parser::FileName> tBoostOptional{input_parser::FileName{std::string{tFileName}}};

    const std::optional<std::string> tStdOptional =
        to_unwrapped_optional(tBoostOptional, [](const auto& aFileName) { return aFileName.mToken; });

    ASSERT_TRUE(tStdOptional);
    EXPECT_EQ(tStdOptional.value(), std::string{tFileName});
}

}  // namespace plato::utilities::unittest
