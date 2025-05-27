#include <gtest/gtest.h>

#include <numbers>
#include <sstream>

#include "plato/utilities/FixedWidthFloatingPointOutput.hpp"

namespace plato::utilities::unittest
{

TEST(FixedWidthFloatingPoinOutput, OutputOperator)
{
    constexpr auto tPrecision = std::size_t{3};
    constexpr auto tWidth = std::size_t{10};
    const auto tPi = FixedWidthFloatingPointOutput<double, tPrecision, tWidth>{std::numbers::pi};

    std::stringstream tString;
    tString << tPi;
    const auto tGold = std::string{"      3.14"};
    EXPECT_EQ(tString.str(), tGold);
}

}  // namespace plato::utilities::unittest
