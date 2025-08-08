#include <gtest/gtest.h>

#include <sstream>

#include "plato/geometry/extension/cubit/BracedOutput.hpp"
#include "plato/utilities/StringUtilities.hpp"

namespace plato::geometry::extension::cubit::unittest
{

TEST(BracedOutput, Output)
{
    constexpr auto kValue = std::string_view{"happy"};
    std::stringstream tStringStream;
    tStringStream << BracedOutput{kValue};
    const auto tResult = tStringStream.str();
    const auto tGold = utilities::concatenate("{", kValue, "}");
    EXPECT_EQ(tResult, tGold);
}

}  // namespace plato::geometry::extension::cubit::unittest
