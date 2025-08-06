#include <gtest/gtest.h>

#include <sstream>

#include "plato/test_utilities/CoutCerrPrintTestFixture.hpp"
#include "plato/utilities/Colorize.hpp"

namespace plato::utilities::unittest
{
namespace
{
class ColorizeRedirectFixture : public test_utilities::CoutCerrPrintTestFixture
{
};

struct AlwaysUseColorPolicy
{
    [[nodiscard]] auto operator()(const std::ostream&) const -> bool { return true; };
};
}  // namespace

TEST(Colorize, StreamInsertionNoColorSupport)
{
    auto tStream = std::stringstream{};
    tStream << colorize("Hello green world!", TextColor::kGreen);
    tStream << colorize(42, TextColor::kRed);
    // Result should not contain any escape characters
    EXPECT_EQ(tStream.str(), "Hello green world!42");
}

TEST_F(ColorizeRedirectFixture, ForcedColorCheck)
{
    std::cout << colorize<std::string, AlwaysUseColorPolicy>("T", TextColor::kRed);
    std::cout << colorize<std::string, AlwaysUseColorPolicy>("a", TextColor::kGreen);
    std::cout << colorize<std::string, AlwaysUseColorPolicy>("s", TextColor::kBlue);
    std::cout << colorize<std::string, AlwaysUseColorPolicy>("t", TextColor::kCyan);
    std::cout << colorize<std::string, AlwaysUseColorPolicy>("e", TextColor::kMagenta);
    std::cout << colorize<std::string, AlwaysUseColorPolicy>("T", TextColor::kYellow);
    std::cout << colorize<std::string, AlwaysUseColorPolicy>("h", TextColor::kBlack);
    std::cout << colorize<std::string, AlwaysUseColorPolicy>("e", TextColor::kLightRed);
    std::cerr << colorize<std::string, AlwaysUseColorPolicy>("R", TextColor::kLightGreen);
    std::cerr << colorize<std::string, AlwaysUseColorPolicy>("a", TextColor::kLightBlue);
    std::cerr << colorize<std::string, AlwaysUseColorPolicy>("i", TextColor::kLightCyan);
    std::cerr << colorize<std::string, AlwaysUseColorPolicy>("n", TextColor::kLightMagenta);
    std::cerr << colorize<std::string, AlwaysUseColorPolicy>("b", TextColor::kLightYellow);
    std::cerr << colorize<std::string, AlwaysUseColorPolicy>("o", TextColor::kLightGray);
    std::cerr << colorize<std::string, AlwaysUseColorPolicy>("w", TextColor::kDarkGray) << "\n";
    std::cout << "Back to normal on cout\n";
    std::cerr << "Back to normal on cerr\n";

    auto tExpected = std::string{"\033[31mT\033[39m"};  // Red: 31
    tExpected += "\033[32ma\033[39m";                   // Green: 32
    tExpected += "\033[34ms\033[39m";                   // Blue: 34
    tExpected += "\033[36mt\033[39m";                   // Cyan: 36
    tExpected += "\033[35me\033[39m";                   // Magenta: 35
    tExpected += "\033[33mT\033[39m";                   // Yellow: 33
    tExpected += "\033[30mh\033[39m";                   // Black: 30
    tExpected += "\033[91me\033[39m";                   // Light red: 91
    tExpected += "Back to normal on cout\n";
    checkRankZeroCoutStringStream(tExpected, TEST_CONTEXT("Cout"));

    tExpected.clear();
    tExpected += "\033[92mR\033[39m";    // Light green: 92
    tExpected += "\033[94ma\033[39m";    // Light blue: 94
    tExpected += "\033[96mi\033[39m";    // Light cyan: 96
    tExpected += "\033[95mn\033[39m";    // Light magenta: 95
    tExpected += "\033[93mb\033[39m";    // Light yellow: 93
    tExpected += "\033[37mo\033[39m";    // Light gray: 37
    tExpected += "\033[90mw\033[39m\n";  // Dark gray: 90
    tExpected += "Back to normal on cerr\n";
    checkRankZeroCerrStringStream(tExpected, TEST_CONTEXT("Cerr"));
}
}  // namespace plato::utilities::unittest
