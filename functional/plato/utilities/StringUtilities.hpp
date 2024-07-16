#ifndef PLATO_UTILITIES_STRINGUTILTIES
#define PLATO_UTILITIES_STRINGUTILTIES

#include <numeric>
#include <string>
#include <string_view>
#include <vector>

namespace plato::utilities
{
/// @brief Concatenates all messages in @a aStrings with a delimeter @a aDelimiter in between each string.
template <template <class...> typename Container>
std::string concatenate_container(const Container<std::string>& aStrings, std::string_view aDelimiter = " ");

/// @brief Concatenates all of @a strings into a single string.
/// @tparam StringConvertible Any type that std::string is constructable from or any arithmetic type that
///  can be converted to a string with `std::to_string`.
template <typename... StringConvertible>
[[nodiscard]] std::string concatenate(StringConvertible&&... strings);

namespace detail
{
template <typename T>
[[nodiscard]] decltype(auto) convert_to_string(T&& t)
{
    using DecayedT = std::decay_t<T>;
    if constexpr (std::is_arithmetic_v<DecayedT> && !std::is_same_v<char, DecayedT>)
    {
        return std::to_string(t);
    }
    else if constexpr (std::is_same_v<DecayedT, std::string>)
    {
        return std::forward<T>(t);
    }
    else
    {
        return std::string{std::forward<T>(t)};
    }
}
}  // namespace detail

template <template <class...> typename Container>
std::string concatenate_container(const Container<std::string>& aStrings, const std::string_view aDelimiter)
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
std::string concatenate(StringConvertible&&... strings)
{
    return (detail::convert_to_string(std::forward<StringConvertible>(strings)) + ...);
}

}  // namespace plato::utilities
#endif
