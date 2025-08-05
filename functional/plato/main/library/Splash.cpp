#include "plato/main/library/Splash.hpp"

#include "plato/utilities/Colorize.hpp"

namespace plato::main::library
{
namespace
{
template <utilities::TextColor kTextColor>
struct ColorizedLogo
{
    std::string_view aText;
};

template <utilities::TextColor kTextColor>
auto operator<<(std::ostream& aStream, const ColorizedLogo<kTextColor> aBorder) -> std::ostream&
{
    aStream << utilities::colorize(aBorder.aText, kTextColor);
    return aStream;
}

using Border = ColorizedLogo<utilities::TextColor::kDarkGray>;
using Logo = ColorizedLogo<utilities::TextColor::kLightCyan>;
using DropShadow = ColorizedLogo<utilities::TextColor::kLightBlue>;
using HelpText = ColorizedLogo<utilities::TextColor::kLightYellow>;
using SubTitleText = ColorizedLogo<utilities::TextColor::kLightBlue>;

}  // namespace

void print_splash_screen(std::ostream& aStream)
{
    // clang-format off
    aStream << Border{"################################################################################\n"};
    aStream << Border{"#                                                                              #\n"};
    aStream << Border{"#"} << Logo{ "                    @@@@@                                                     "} << Border{"#\n"};
    aStream << Border{"#"} << Logo{"                    @@@@@                                                     "} << Border{"#\n"};
    aStream << Border{"#"} << Logo{"                    @@@@@                                                     "} << Border{"#\n"};
    aStream << Border{"#"} << Logo{"                    @@@@@                       ,*****                        "} << Border{"#\n"};
    aStream << Border{"#"} << Logo{"                    @@@@@                       "} << DropShadow{"%"} << Logo{"@@@@@                        "} << Border{"#\n"};
    aStream << Border{"#"} << Logo{"                    @@@@@                       "} << DropShadow{"%"} << Logo{"@@@@@                        "} << Border{"#\n"};
    aStream << Border{"#"} << Logo{"                    @@@@@                       "} << DropShadow{"%"} << Logo{"@@@@@                        "} << Border{"#\n"};
    aStream << Border{"#"} << Logo{"  @@@@@@@@@@@@@@@   @@@@@    @@@@@@@@@@@@@@   @@@@@@@@@@@   @@@@@@@@@@@@@@@,  "} << Border{"#\n"};
    aStream << Border{"#"} << Logo{"  @@@@@@@@@@@@@@@   @@@@@   "} << DropShadow{","} << Logo{"@@@@@@@@@@@@@@   @@@@@@@@@@@   @@@@@@@@@@@@@@@@  "} << Border{"#\n"};
    aStream << Border{"#"} << Logo{"  @@@@@     @@@@@   @@@@@   "} << DropShadow{","} << Logo{"@@@@     @@@@@     "} << DropShadow{"%"} << Logo{"@@@@@      @@@@@.    "} << DropShadow{"%"} << Logo{"@@@@@  "} << Border{"#\n"};
    aStream << Border{"#"} << Logo{"  @@@@@     @@@@@   @@@@@             @@@@@     "} << DropShadow{"%"} << Logo{"@@@@@      @@@@@.    "} << DropShadow{"%"} << Logo{"@@@@@  "} << Border{"#\n"};
    aStream << Border{"#"} << Logo{"  @@@@@     @@@@@   @@@@@    "} << DropShadow{"*"} << Logo{"@@@@@@@@@@@@@     "} << DropShadow{"%"} << Logo{"@@@@@      @@@@@.    "} << DropShadow{"%"} << Logo{"@@@@@  "} << Border{"#\n"};
    aStream << Border{"#"} << Logo{"  @@@@@     @@@@@   @@@@@   @@@@@@@@@@@@@@@     "} << DropShadow{"%"} << Logo{"@@@@@      @@@@@.    "} << DropShadow{"%"} << Logo{"@@@@@  "} << Border{"#\n"};
    aStream << Border{"#"} << Logo{"  @@@@@     @@@@@   @@@@@   @@@@@     @@@@@     "} << DropShadow{"%"} << Logo{"@@@@@      @@@@@.    "} << DropShadow{"%"} << Logo{"@@@@@  "} << Border{"#\n"};
    aStream << Border{"#"} << Logo{"  @@@@@     @@@@@   @@@@@   @@@@@     @@@@@     "} << DropShadow{"%"} << Logo{"@@@@@      @@@@@.    "} << DropShadow{"%"} << Logo{"@@@@@  "} << Border{"#\n"};
    aStream << Border{"#"} << Logo{"  @@@@@     @@@@@   @@@@@   @@@@@     @@@@@     "} << DropShadow{"%"} << Logo{"@@@@@      @@@@@.    "} << DropShadow{"%"} << Logo{"@@@@@  "} << Border{"#\n"};
    aStream << Border{"#"} << Logo{"  @@@@@@@@@@@@@@@   @@@@@   @@@@@@@@@@@@@@@     "} << DropShadow{"%"} << Logo{"@@@@@@@@   @@@@@@@@@@@@@@@@  "} << Border{"#\n"};
    aStream << Border{"#"} << Logo{"  @@@@@@@@@@@@@@#   @@@@@   "} << DropShadow{"&"} << Logo{"@@@@@@@@@@@@@@      @@@@@@@@   @@@@@@@@@@@@@@@*  "} << Border{"#\n"};
    aStream << Border{"#"} << Logo{"  @@@@@                                                                       "} << Border{"#\n"};
    aStream << Border{"#"} << Logo{"  @@@@@                                                                       "} << Border{"#\n"};
    aStream << Border{"#"} << Logo{"  @@@@@   "} << SubTitleText{"OPTIMIZATION-BASED DESIGN"} << "                                           " << Border{"#\n"};
    aStream << Border{"#"} << Logo{"  @@@@@                                                                       "} << Border{"#\n"};
    aStream << Border{"#"} << Logo{"  @@@@@   "} << HelpText{"Questions? contact Plato3d-help@sandia.gov"} << "                          " << Border{"#\n"};
    aStream << Border{"#"} << Logo{ "                                                                              "} << Border{"#\n"};
    aStream << Border{"################################################################################\n\n"};
    // clang-format on
}
}  // namespace plato::main::library
