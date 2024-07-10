#ifndef PLATO_UTILITIES_HASHGENERATION
#define PLATO_UTILITIES_HASHGENERATION

#include <boost/functional/hash.hpp>
#include <cstddef>

namespace plato::core
{
struct MeshProxy;
}

namespace plato::filter::library
{
[[nodiscard]] std::size_t hash_mesh(const plato::mesh::MeshProxy& aMeshProxy);

namespace detail
{
template <typename ContainerType>
[[nodiscard]] std::size_t hash_container(const ContainerType& aContainer)
{
    std::size_t tSeed = 0;
    for (const auto& aVal : aContainer)
    {
        boost::hash_combine(tSeed, aVal);
    }
    return tSeed;
}
}  // namespace detail

}  // namespace plato::filter::library

#endif