#ifndef PLATO_UTILITIES_CONTAINERHELPERS
#define PLATO_UTILITIES_CONTAINERHELPERS

#include <cstddef>

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

template <ReservableContainer Container>
[[nodiscard]] constexpr auto reserved_container(const std::size_t aCapacity) -> Container
{
    auto tContainer = Container{};
    tContainer.reserve(aCapacity);
    return tContainer;
}
}  // namespace plato::utilities

#endif
