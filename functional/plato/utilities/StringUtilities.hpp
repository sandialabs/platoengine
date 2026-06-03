#ifndef PLATO_UTILITIES_STRINGUTILTIES
#define PLATO_UTILITIES_STRINGUTILTIES

#include <numeric>
#include <string>
#include <string_view>

namespace plato::utilities
{
/// @brief Concatenates all messages in @a aStrings with a delimeter @a aDelimiter in between each string.
template <template <class...> typename Container>
auto concatenate_container(const Container<std::string>& aStrings, std::string_view aDelimiter = " ") -> std::string;

/// @brief Concatenates all of @a strings into a single string.
/// @tparam StringConvertible Any type that std::string is constructable from or any arithmetic type that
///  can be converted to a string with `std::to_string`.
template <typename... StringConvertible>
[[nodiscard]] auto concatenate(StringConvertible&&... aStrings) -> std::string;

namespace detail
{
template <typename T>
[[nodiscard]] auto convert_to_string(T&& aVariableToConvert) -> decltype(auto)
{
    using DecayedT = std::remove_cvref_t<T>;
    if constexpr (std::is_arithmetic_v<DecayedT> && !std::is_same_v<char, DecayedT>)
    {
        return std::to_string(aVariableToConvert);
    }
    else if constexpr (std::is_same_v<DecayedT, std::string>)
    {
        return std::forward<T>(aVariableToConvert);
    }
    else if constexpr (std::is_array_v<DecayedT> && std::is_same_v<char, std::remove_extent_t<DecayedT>>)
    {
        return std::string(std::data(aVariableToConvert));
    }
    else
    {
        return std::string{std::forward<T>(aVariableToConvert)};
    }
}
}  // namespace detail

template <template <class...> typename Container>
auto concatenate_container(const Container<std::string>& aStrings, const std::string_view aDelimiter) -> std::string
{
    if (aStrings.empty())
    {
        return {};
    }
    return std::accumulate(std::next(aStrings.cbegin()), aStrings.cend(), *aStrings.cbegin(),
                           [aDelimiter](std::string aAllNames, std::string aCurrentName)
                           { return std::move(aAllNames) + std::string{aDelimiter} + std::move(aCurrentName); });
}

template <typename... StringConvertible>
auto concatenate(StringConvertible&&... aStrings) -> std::string
{
    return (detail::convert_to_string(std::forward<StringConvertible>(aStrings)) + ...);
}

}  // namespace plato::utilities
#endif
