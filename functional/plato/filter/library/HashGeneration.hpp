#ifndef PLATO_FILTER_LIBRARY_HASHGENERATION
#define PLATO_FILTER_LIBRARY_HASHGENERATION

#include <boost/functional/hash.hpp>
#include <cstddef>

namespace plato::design_variables
{
struct MeshDesignVariables;
}

namespace plato::filter::library
{
/// @brief Computes a hash of the nodal coordinates of the mesh referenced by the file name in
///  @a aMeshDesignVariables. Does not consider the density values or connectivity, only the nodal coordinates.
[[nodiscard]] std::size_t hash_mesh_coordinates(
    const plato::design_variables::MeshDesignVariables& aMeshDesignVariables);

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
