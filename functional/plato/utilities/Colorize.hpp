#ifndef PLATO_UTILITIES_COLORIZE
#define PLATO_UTILITIES_COLORIZE

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

/// @brief Constraint for types that have a stream insertion operator overload.
template <typename T>
concept Streamable = requires(const T& aT, std::ostream& aStream) { aStream << aT; };

/// @brief Struct to tie some text with a color, may be used with the stream insertion operator to conditionally
/// colorize text for console output.
template <Streamable T>
struct Colorize
{
    const T& mObjectToStream;
    TextColor mTextColor;
};

/// @brief Helper function for deducing the object type. This may be more concise than using the struct directly.
template <Streamable T>
[[nodiscard]] auto colorize(const T& aObjectToStream, TextColor aColor) -> Colorize<T>;

/// @brief Stream insertion operator for ColorizedText.
template <typename T>
auto operator<<(std::ostream& aStream, Colorize<T> aColorizedText) -> std::ostream&;

namespace detail
{
/// @brief Checks if @a aStream is capable of handling color output.
[[nodiscard]] auto stream_supports_color(const std::ostream& aStream) -> bool;

/// @brief Returns the escape sequence and color code associated with @a aTextColor.
[[nodiscard]] auto color_code(const TextColor aTextColor) -> std::string;
}  // namespace detail

template <typename T>
auto operator<<(std::ostream& aStream, Colorize<T> aColorizedText) -> std::ostream&
{
    if (detail::stream_supports_color(aStream))
    {
        aStream << detail::color_code(aColorizedText.mTextColor) << aColorizedText.mObjectToStream
                << detail::color_code(TextColor::kDefault);
    }
    else
    {
        aStream << aColorizedText.mObjectToStream;
    }
    return aStream;
}

template <Streamable T>
[[nodiscard]] auto colorize(const T& aObjectToStream, const TextColor aColor) -> Colorize<T>
{
    return Colorize<T>{aObjectToStream, aColor};
}

}  // namespace plato::utilities

#endif
