#include <gtest/gtest.h>

#include <sstream>

#include "plato/utilities/ColorizedText.hpp"

namespace plato::utilities::unittest
{
TEST(ColorizedText, StreamInsertionNoColorSupport)
{
    auto tStream = std::stringstream{};
    tStream << ColorizedText{.mText = "Hello green world!", .mTextColor = TextColor::kGreen};
    // Result should not contain any escape characters
    EXPECT_EQ(tStream.str(), "Hello green world!");
}

TEST(ColorizedText, ManualColorCheck)
{
    GTEST_SKIP() << "Comment this out and run manually to check the colors in the terminal";

    std::cout << ColorizedText{.mText = "T", .mTextColor = TextColor::kRed};
    std::cout << ColorizedText{.mText = "a", .mTextColor = TextColor::kGreen};
    std::cout << ColorizedText{.mText = "s", .mTextColor = TextColor::kBlue};
    std::cout << ColorizedText{.mText = "t", .mTextColor = TextColor::kCyan};
    std::cout << ColorizedText{.mText = "e", .mTextColor = TextColor::kMagenta};
    std::cout << ColorizedText{.mText = "T", .mTextColor = TextColor::kYellow};
    std::cout << ColorizedText{.mText = "h", .mTextColor = TextColor::kBlack};
    std::cout << ColorizedText{.mText = "e", .mTextColor = TextColor::kLightRed};
    std::cerr << ColorizedText{.mText = "R", .mTextColor = TextColor::kLightGreen};
    std::cerr << ColorizedText{.mText = "a", .mTextColor = TextColor::kLightBlue};
    std::cerr << ColorizedText{.mText = "i", .mTextColor = TextColor::kLightCyan};
    std::cerr << ColorizedText{.mText = "n", .mTextColor = TextColor::kLightMagenta};
    std::cerr << ColorizedText{.mText = "b", .mTextColor = TextColor::kLightYellow};
    std::cerr << ColorizedText{.mText = "o", .mTextColor = TextColor::kLightGray};
    std::cerr << ColorizedText{.mText = "w", .mTextColor = TextColor::kDarkGray} << "\n";
    std::cout << "Back to normal on cout\n";
    std::cout << "Back to normal on cerr\n";
}
}  // namespace plato::utilities::unittest
