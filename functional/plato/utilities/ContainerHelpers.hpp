#ifndef PLATO_UTILITIES_CONTAINERHELPERS
#define PLATO_UTILITIES_CONTAINERHELPERS

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <optional>
#include <utility>

namespace plato::utilities
{
/// @brief Concept for types with a `reserve` member function.
template <typename Container>
concept ReservableContainer = requires(Container aContainer) { aContainer.reserve(std::size_t{}); };

/// @brief Constructs an empty container with its capacity set to @a aCapacity.
///
/// Calls the reserve member of the Container type.
template <ReservableContainer Container>
[[nodiscard]] constexpr auto reserved_container(std::size_t aCapacity) -> Container;

/// @brief Requirements for a map-like data structure.
template <typename Container>
concept MapLike = requires {
    typename Container::value_type;
    typename Container::key_type;
    typename Container::mapped_type;
    requires std::same_as<typename Container::value_type,
                          std::pair<const typename Container::key_type, typename Container::mapped_type>>;
};

/// @brief Finds the first instance of @a aValue in a map-like data structure.
template <typename Container, typename U>
    requires MapLike<Container> && std::convertible_to<U, typename Container::mapped_type>
[[nodiscard]] auto find_key_with_value(const Container& aContainer,
                                       const U& aMappedValue) -> std::optional<typename Container::key_type>;

template <ReservableContainer Container>
[[nodiscard]] constexpr auto reserved_container(const std::size_t aCapacity) -> Container
{
    auto tContainer = Container{};
    tContainer.reserve(aCapacity);
    return tContainer;
}

template <typename Container, typename U>
    requires MapLike<Container> && std::convertible_to<U, typename Container::mapped_type>
[[nodiscard]] auto find_key_with_value(const Container& aContainer,
                                       const U& aMappedValue) -> std::optional<typename Container::key_type>
{
    const auto tFoundValue =
        std::ranges::find_if(aContainer, [&aMappedValue](const auto& aEntry) { return aEntry.second == aMappedValue; });
    if (tFoundValue == aContainer.end())
    {
        return std::nullopt;
    }
    return std::make_optional(tFoundValue->first);
}

}  // namespace plato::utilities

#endif
