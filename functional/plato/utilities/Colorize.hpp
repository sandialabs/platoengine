#ifndef PLATO_UTILITIES_COLORIZE
#define PLATO_UTILITIES_COLORIZE

#include <concepts>
#include <cstdint>
#include <ostream>
#include <string_view>

namespace plato::utilities
{
/// @brief Text color for console output.
/// @sa Colorize
/// @sa colorize
enum struct TextColor : std::uint8_t
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

/// @brief Checks if @a aStream is capable of handling color output.
[[nodiscard]] auto stream_supports_color(const std::ostream& aStream) -> bool;

/// @brief Returns the escape sequence and color code associated with @a aTextColor.
[[nodiscard]] auto color_code(const TextColor aTextColor) -> std::string;

namespace detail
{
/// @brief Concept for the policy that will be checked to determine if color output should be used.
template <typename F>
concept UseColorPolicy = requires(F aF, const std::ostream& aStream) {
    {
        aF(aStream)
    } -> std::convertible_to<bool>;
};

/// @brief The default policy to use for Colorize and colorize, checks @a aStream with `isatty`.
struct DefaultColorPolicy
{
    [[nodiscard]] auto operator()(const std::ostream& aStream) -> bool;
};
}  // namespace detail

/// @brief Constraint for types that have a stream insertion operator overload.
template <typename T>
concept Streamable = requires(const T& aT, std::ostream& aStream) { aStream << aT; };

/// @brief Struct to tie some text with a color, may be used with the stream insertion operator to conditionally
/// colorize text for console output.
template <Streamable T, detail::UseColorPolicy ColorPolicy = detail::DefaultColorPolicy>
struct Colorize
{
    const T& mObjectToStream;
    TextColor mTextColor;
};

/// @brief Helper function for deducing the object type. This may be more concise than using the struct directly.
template <Streamable T, detail::UseColorPolicy ColorPolicy = detail::DefaultColorPolicy>
[[nodiscard]] auto colorize(const T& aObjectToStream, TextColor aColor) -> Colorize<T, ColorPolicy>;

/// @brief Stream insertion operator for ColorizedText.
template <typename T, typename ColorPolicy>
auto operator<<(std::ostream& aStream, Colorize<T, ColorPolicy> aColorizedText) -> std::ostream&;

template <typename T, typename ColorPolicy>
auto operator<<(std::ostream& aStream, Colorize<T, ColorPolicy> aColorizedText) -> std::ostream&
{
    if (ColorPolicy{}(aStream))
    {
        aStream << color_code(aColorizedText.mTextColor) << aColorizedText.mObjectToStream
                << color_code(TextColor::kDefault);
    }
    else
    {
        aStream << aColorizedText.mObjectToStream;
    }
    return aStream;
}

template <Streamable T, detail::UseColorPolicy ColorPolicy>
[[nodiscard]] auto colorize(const T& aObjectToStream, const TextColor aColor) -> Colorize<T, ColorPolicy>
{
    return Colorize<T, ColorPolicy>{aObjectToStream, aColor};
}

}  // namespace plato::utilities

#endif
