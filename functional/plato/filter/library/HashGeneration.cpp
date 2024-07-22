#include "plato/filter/library/HashGeneration.hpp"

#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshProxy.hpp"

namespace plato::filter::library
{
std::size_t hash_mesh_coordinates(const plato::mesh::MeshProxy& aMeshProxy)
{
    const auto tCoords = mesh::EntityRetrieval{mesh::Mesh{aMeshProxy.mFileName}}.flattenedNodalCoordinates();
    return detail::hash_container(tCoords);
}

}  // namespace plato::filter::library