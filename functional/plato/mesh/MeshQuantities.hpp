#ifndef PLATO_MESH_MESHQUANTITIES
#define PLATO_MESH_MESHQUANTITIES

#include "plato/mesh/Mesh.hpp"

namespace plato::mesh
{
/// @brief A mixin extension for Mesh, this provides functions that compute global mesh quantities such as volume.
struct MeshQuantities : public Mesh
{
    MeshQuantities(Mesh aMeshBase);

    /// @brief Returns the total volume of the mesh.
    [[nodiscard]] double volume() const;

    /// @brief Returns the average nodal density of the mesh, computed as the total number of nodes divided by the
    /// volume.
    [[nodiscard]] double averageNodalDensity() const;
};
}  // namespace plato::mesh

#endif
