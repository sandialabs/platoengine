#ifndef PLATO_MESH_DESIGNVARIABLECONVERSION
#define PLATO_MESH_DESIGNVARIABLECONVERSION

#include <vector>

#include "plato/mesh/MeshProxy.hpp"

namespace plato::mesh
{
class Mesh;
}

namespace plato::mesh
{
/// @brief Maps the nodal densities in @a aDensities to a MeshProxy object using the node, element, and block info in @a
/// aMesh.
///
/// @pre The size of @a aDensities must be equal to the total number of nodes in @a aMesh
/// @post The ordering of the densities in the returned MeshProxy will be such that iterating over the MeshProxy with
/// MeshProxyDensitiesView will match the order of @a aDensities. In other words, the following assertions are true:
/// @code{.cpp}
/// assert(MeshProxyDensitiesView{aMesh}.size(), aDensities.size());
/// for(const auto& [aMeshDensity, aVectorDensityValue] : Zip{MeshProxyDensitiesView{aMesh}, aDensities})
///   assert(aMeshDensity.mDensity == aVectorDensityValue);
/// @endcode
MeshProxy nodal_densities_to_mesh_proxy(const std::vector<double>& aDensities, const Mesh& aMesh);

/// @brief Maps the element densities in @a aDensities to a MeshProxy object using the node, element, and block info in
/// @a aMesh.
///
/// @pre The size of @a aDensities must be equal to the total number of elements in @a aMesh
/// @post The ordering of the densities in the returned MeshProxy will be such that iterating over the MeshProxy with
/// MeshProxyDensitiesView will match the order of @a aDensities. In other words, the following assertions are true:
/// @code{.cpp}
/// assert(MeshProxyDensitiesView{aMesh}.size(), aDensities.size());
/// for(const auto& [aMeshDensity, aVectorDensityValue] : Zip{MeshProxyDensitiesView{aMesh}, aDensities})
///   assert(aMeshDensity.mDensity == aVectorDensityValue);
/// @endcode
MeshProxy element_densities_to_mesh_proxy(const std::vector<double>& aDensities, const Mesh& aMesh);

}  // namespace plato::mesh

#endif
