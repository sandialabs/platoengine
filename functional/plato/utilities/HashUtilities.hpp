#ifndef PLATO_UTILITIES_HASHUTILITIES
#define PLATO_UTILITIES_HASHUTILITIES

#include <boost/functional/hash.hpp>
#include <numeric>

namespace plato::utilities
{
/// @brief Computes a hash of the contents of @a aContainer.
///
/// The type contained in the container must provide an overload for the function `hash_value` available via ADL.
template <typename ContainerType>
[[nodiscard]] auto hash_container(const ContainerType& aContainer) -> std::size_t;

template <typename ContainerType>
auto hash_container(const ContainerType& aContainer) -> std::size_t
{
    return std::accumulate(aContainer.cbegin(), aContainer.cend(), std::size_t{0},
                           [](std::size_t aSeed, const auto& aEntry)
                           {
                               boost::hash_combine(aSeed, aEntry);
                               return aSeed;
                           });
}
}  // namespace plato::utilities

#endif
