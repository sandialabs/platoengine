#ifndef PLATO_LINEAR_ALGEBRA_DYNAMICVECTORFORMATTER
#define PLATO_LINEAR_ALGEBRA_DYNAMICVECTORFORMATTER

#include <format>
#include <ranges>

#include "plato/linear_algebra/DynamicVector.hpp"

namespace plato::linear_algebra::detail
{
/// @brief Parses an iterator range to an unsigned integer.
///
/// @pre The iterator range must be valid (@a aLast is reachable from @a aFirst ).
/// @return `std::nullopt` on any conversion errors, such as the char range not containing only numerical digits.
template <typename Iterator, typename CharType = std::decay_t<decltype(*std::declval<Iterator>())>>
    requires requires {
        { *std::declval<Iterator>() } -> std::convertible_to<CharType>;
    }
[[nodiscard]] constexpr auto to_int(Iterator aFirst, const Iterator aLast) -> std::optional<std::size_t>;
}  // namespace plato::linear_algebra::detail

/// @brief Specialization of `std::formatter` for DynamicVector
///
/// This is used so that a DynamicVector can be used with `std::format`. Its formatting spec is of the form
/// `{:N:spec}`, where `N` is an optional maximum size to print. The spec after the second colon `:` is the spec used to
/// format the container entries. The second colon is still required even if no size is provided, e.g. `{::9.8e}`.
template <typename T, typename CharType>
    requires requires { std::formatter<T, CharType>{}; }
class std::formatter<plato::linear_algebra::DynamicVector<T>, CharType>
{
   public:
    constexpr auto parse(std::basic_format_parse_context<CharType>& aContext);
    auto format(const plato::linear_algebra::DynamicVector<T>& aDynamicVector, auto& aContext) const;

   private:
    std::formatter<T> mEntryFormatter;
    std::optional<std::size_t> mMaximumEntries = std::nullopt;
};

namespace plato::linear_algebra::detail
{
template <typename Iterator, typename CharType>
    requires requires {
        { *std::declval<Iterator>() } -> std::convertible_to<CharType>;
    }
constexpr auto to_int(Iterator aFirst, const Iterator aLast) -> std::optional<std::size_t>
{
    if (std::any_of(aFirst, aLast, [](const auto aChar) { return aChar < '0' || aChar > '9'; }))
    {
        return std::nullopt;
    }

    return std::accumulate(aFirst, aLast, std::size_t{0U},
                           [](const std::size_t aValue, const auto aChar) { return aValue * 10 + (aChar - '0'); });
}
}  // namespace plato::linear_algebra::detail

template <typename T, typename CharType>
    requires requires { std::formatter<T, CharType>{}; }
constexpr auto std::formatter<plato::linear_algebra::DynamicVector<T>, CharType>::parse(
    std::basic_format_parse_context<CharType>& aContext)
{
    if (*aContext.begin() != ':' && *aContext.begin() != '}')
    {
        const auto tSpecIterator = aContext.begin();
        const auto tSpecEndIterator = std::find_if(
            tSpecIterator, aContext.end(), [](const auto aSpecChar) { return aSpecChar == ':' || aSpecChar == '}'; });
        mMaximumEntries = plato::linear_algebra::detail::to_int(tSpecIterator, tSpecEndIterator);
        aContext.advance_to(tSpecEndIterator);
    }
    if (*aContext.begin() == ':')
    {
        aContext.advance_to(aContext.begin() + 1);
        return mEntryFormatter.parse(aContext);
    }
    return aContext.begin();
}

template <typename T, typename CharType>
    requires requires { std::formatter<T, CharType>{}; }
auto std::formatter<plato::linear_algebra::DynamicVector<T>, CharType>::format(
    const plato::linear_algebra::DynamicVector<T>& aDynamicVector, auto& aContext) const
{
    if (mMaximumEntries.value_or(aDynamicVector.size()) == 0U)
    {
        return std::format_to(aContext.out(), "");
    }

    const auto tEntryLimit = mMaximumEntries.value_or(aDynamicVector.size());
    const auto tMaximumEntriesToWrite = std::min(tEntryLimit, aDynamicVector.size());
    const auto tEntries = aDynamicVector.stdVector() | std::views::take(tMaximumEntriesToWrite - 1);
    for (const auto& tEntry : tEntries)
    {
        aContext.advance_to(mEntryFormatter.format(tEntry, aContext));
        aContext.advance_to(std::format_to(aContext.out(), ", "));
    }
    if (aDynamicVector.size() > tEntryLimit)
    {
        aContext.advance_to(mEntryFormatter.format(aDynamicVector.stdVector().at(tEntryLimit - 1), aContext));
        aContext.advance_to(std::format_to(aContext.out(), ", ..."));
    }
    else
    {
        aContext.advance_to(mEntryFormatter.format(aDynamicVector.stdVector().back(), aContext));
    }
    return aContext.out();
}

#endif
