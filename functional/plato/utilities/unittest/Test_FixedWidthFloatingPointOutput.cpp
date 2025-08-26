#include <gtest/gtest.h>

#include <numbers>
#include <sstream>

#include "plato/utilities/FixedWidthFloatingPointOutput.hpp"

namespace plato::utilities::unittest
{

TEST(FixedWidthFloatingPointOutput, OutputOperator)
{
    constexpr auto tPrecision = std::size_t{3};
    constexpr auto tWidth = std::size_t{10};
    const auto tPi = FixedWidthFloatingPointOutput<double, tPrecision, tWidth>{std::numbers::pi};

    std::stringstream tString;
    tString << tPi;
    const auto tGold = std::string{"      3.14"};
    EXPECT_EQ(tString.str(), tGold);
}

TEST(FixedWidthFloatingPointOutput, ToString)
{
    constexpr auto tPrecision = std::size_t{4};
    constexpr auto tWidth = std::size_t{6};
    const auto tE = FixedWidthFloatingPointOutput<double, tPrecision, tWidth>{std::numbers::e};

    const auto tResult = to_string(tE);
    constexpr auto tExpected = std::string_view{" 2.718"};
    EXPECT_EQ(tResult, tExpected);
}

}  // namespace plato::utilities::unittest
