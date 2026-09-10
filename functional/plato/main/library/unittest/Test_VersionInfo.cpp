#include <gtest/gtest.h>

#include <iterator>
#include <regex>

#include "plato/main/library/VersionInfo.hpp"

namespace plato::main::library::unittest
{

TEST(VersionInfo, VersionInfo)
{
    const auto tVersionInfo = version_info();
    const auto tPattern = std::regex{"Version [0-9].[0-9].[0-9] : commit"};
    const auto tPatternOccurrence =
        std::distance(std::sregex_iterator{tVersionInfo.begin(), tVersionInfo.end(), tPattern}, std::sregex_iterator{});
    constexpr auto tOnce = 1U;
    EXPECT_EQ(tPatternOccurrence, tOnce);
}

}  // namespace plato::main::library::unittest
