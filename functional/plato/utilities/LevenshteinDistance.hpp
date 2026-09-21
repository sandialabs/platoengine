#ifndef PLATO_INPUTPARSER_LEVENSHTEINDISTANCE
#define PLATO_INPUTPARSER_LEVENSHTEINDISTANCE

#include <algorithm>
#include <cstddef>
#include <functional>
#include <ranges>
#include <string_view>
#include <type_traits>

namespace plato::utilities
{

/// @brief Sorts @a aContainer by Levenshtein distance from @a aToken using @a aKeyAccessor as the comparison key.
///
/// @a aKeyAccessor must return a string-like value accepted by `std::string_view`.
template <std::ranges::random_access_range Container, typename KeyAccessor>
    requires(std::is_invocable_r_v<std::string_view, KeyAccessor, const typename Container::value_type&>)
auto levenshtein_distance_sorted_container(Container aContainer,
                                           std::string_view aToken,
                                           const KeyAccessor& aKeyAccessor) -> Container;

namespace detail
{

/// @brief Compute the Levenshtein Distance between two strings ignoring case
[[nodiscard]] auto levenshtein_distance(std::string_view aLeft, std::string_view aRight) -> std::size_t;

}  // namespace detail

template <std::ranges::random_access_range Container, typename KeyAccessor>
    requires(std::is_invocable_r_v<std::string_view, KeyAccessor, const typename Container::value_type&>)
auto levenshtein_distance_sorted_container(Container aContainer,
                                           std::string_view aToken,
                                           const KeyAccessor& aKeyAccessor) -> Container
{
    std::ranges::stable_sort(aContainer,
                             [aToken, &aKeyAccessor](const auto& aLeft, const auto& aRight)
                             {
                                 const auto tLeftKey = aKeyAccessor(aLeft);
                                 const auto tRightKey = aKeyAccessor(aRight);

                                 const auto tLeftDistance = detail::levenshtein_distance(aToken, tLeftKey);
                                 const auto tRightDistance = detail::levenshtein_distance(aToken, tRightKey);

                                 if (tLeftDistance != tRightDistance)
                                 {
                                     return tLeftDistance < tRightDistance;
                                 }

                                 return tLeftKey < tRightKey;
                             });

    return aContainer;
}

}  // namespace plato::utilities
#endif
