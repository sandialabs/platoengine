#ifndef PLATO_UTILITIES_HASHUTILITIES
#define PLATO_UTILITIES_HASHUTILITIES

#include <boost/functional/hash.hpp>

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
    auto tSeed = std::size_t{0};
    for (const auto& aVal : aContainer)
    {
        boost::hash_combine(tSeed, aVal);
    }
    return tSeed;
}
}  // namespace plato::utilities

#endif
