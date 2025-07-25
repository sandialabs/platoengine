#ifndef PLATO_UTILITIES_COLORTEXT
#define PLATO_UTILITIES_COLORTEXT

#include <ostream>
#include <string_view>

namespace plato::utilities
{
/// @brief Text color for console output.
enum struct TextColor
{
    kDefault,
    kRed,
    kGreen,
    kBlue,
    kCyan,
    kMagenta,
    kYellow,
    kBlack,
    kLightRed,
    kLightGreen,
    kLightBlue,
    kLightCyan,
    kLightMagenta,
    kLightYellow,
    kLightGray,
    kDarkGray,
};

/// @brief Struct to tie some text with a color, may be used with the stream insertion operator to conditionally
/// colorize text for console output.
struct ColorizedText
{
    std::string_view mText;
    TextColor mTextColor;
};

/// @brief Stream insertion operator for ColorizedText.
auto operator<<(std::ostream& aStream, ColorizedText aColorizedText) -> std::ostream&;

}  // namespace plato::utilities

#endif
