#include <gtest/gtest.h>

#include "plato/third_party_integration/boost_log/FormattingUtilities.hpp"
#include "plato/utilities/Colorize.hpp"

namespace plato::third_party_integration::boost_log::unittest
{
TEST(FormattingUtilities, ColorCode)
{
    EXPECT_TRUE(color_code(utilities::TextColor::kRed, FormattingStyle::kNone).empty());

    EXPECT_EQ(color_code(utilities::TextColor::kRed, FormattingStyle::kColor),
              utilities::color_code(utilities::TextColor::kRed));
}
}  // namespace plato::third_party_integration::boost_log::unittest
