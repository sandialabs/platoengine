#include <gtest/gtest.h>

#include <sstream>

#include "plato/utilities/Colorize.hpp"

namespace plato::utilities::unittest
{
TEST(Colorize, StreamInsertionNoColorSupport)
{
    auto tStream = std::stringstream{};
    tStream << colorize("Hello green world!", TextColor::kGreen);
    tStream << colorize(42, TextColor::kRed);
    // Result should not contain any escape characters
    EXPECT_EQ(tStream.str(), "Hello green world!42");
}

TEST(Colorize, ManualColorCheck)
{
    GTEST_SKIP() << "Comment this out and run manually to check the colors in the terminal";

    std::cout << colorize("T", TextColor::kRed);
    std::cout << colorize("a", TextColor::kGreen);
    std::cout << colorize("s", TextColor::kBlue);
    std::cout << colorize("t", TextColor::kCyan);
    std::cout << colorize("e", TextColor::kMagenta);
    std::cout << colorize("T", TextColor::kYellow);
    std::cout << colorize("h", TextColor::kBlack);
    std::cout << colorize("e", TextColor::kLightRed);
    std::cerr << colorize("R", TextColor::kLightGreen);
    std::cerr << colorize("a", TextColor::kLightBlue);
    std::cerr << colorize("i", TextColor::kLightCyan);
    std::cerr << colorize("n", TextColor::kLightMagenta);
    std::cerr << colorize("b", TextColor::kLightYellow);
    std::cerr << colorize("o", TextColor::kLightGray);
    std::cerr << colorize("w", TextColor::kDarkGray) << "\n";
    std::cout << "Back to normal on cout\n";
    std::cout << "Back to normal on cerr\n";
}
}  // namespace plato::utilities::unittest
