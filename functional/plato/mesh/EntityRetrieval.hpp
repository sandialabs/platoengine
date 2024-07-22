#ifndef PLATO_MESH_ENTITYRETRIEVAL
#define PLATO_MESH_ENTITYRETRIEVAL

#include "plato/mesh/Mesh.hpp"

namespace plato::mesh
{
/// @brief A mixin class for Mesh that provides utilities to retrieve entities from a mesh, such as nodal
/// coordinates.
struct EntityRetrieval : public Mesh
{
    EntityRetrieval(Mesh aMeshBase);

    /// @brief The nodal coordinates ordered as x0,y0,z0,x1,y1,z1
    /// @note For 2D, only x and y coordinates are included in the vector.
    [[nodiscard]] auto flattenedNodalCoordinates() const -> std::vector<double>;

    /// @brief All nodal coordinates in the mesh
    [[nodiscard]] auto nodalCoordinates() const -> std::vector<third_party_integration::common::Coordinate>;

    /// @brief Centroids of all the elements in the mesh.
    [[nodiscard]] auto elementCentroids() const -> std::vector<third_party_integration::common::Coordinate>;
};

}  // namespace plato::mesh

#endif
