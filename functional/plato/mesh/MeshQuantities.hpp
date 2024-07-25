#ifndef PLATO_MESH_MESHQUANTITIES
#define PLATO_MESH_MESHQUANTITIES

#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshDesignVariables.hpp"

namespace plato::mesh
{
/// @brief A mixin extension for Mesh, this provides functions that compute global mesh quantities such as volume.
struct MeshQuantities : public Mesh
{
    explicit MeshQuantities(Mesh aMeshBase);

    /// @brief Returns the total volume of the mesh.
    [[nodiscard]] double volume() const;

    /// @brief Returns the average nodal density of the mesh, computed as the total number of nodes divided by the
    /// volume.
    [[nodiscard]] double averageNodalDensity() const;

    /// @brief Returns a vector of element volumes corresponding to all elements in this mesh.
    ///
    /// The order is given by the order of iteration of MeshDesignVariablesDensitiesView.
    [[nodiscard]] std::vector<double> elementVolumes() const;
};
}  // namespace plato::mesh

#endif
