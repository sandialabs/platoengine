#include "plato/filter/library/HashGeneration.hpp"

#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshDesignVariables.hpp"

namespace plato::filter::library
{
std::size_t hash_mesh_coordinates(const plato::mesh::MeshDesignVariables& aMeshDesignVariables)
{
    const auto tCoords = mesh::EntityRetrieval{mesh::Mesh{aMeshDesignVariables.mFileName}}.flattenedNodalCoordinates();
    return detail::hash_container(tCoords);
}

}  // namespace plato::filter::library