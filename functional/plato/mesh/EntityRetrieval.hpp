#ifndef PLATO_MESH_ENTITYRETRIEVAL
#define PLATO_MESH_ENTITYRETRIEVAL

#include "plato/mesh/Mesh.hpp"
#include "plato/third_party_integration/common/Vector3.hpp"

namespace plato::mesh
{
/// @brief A mixin class for Mesh that provides utilities to retrieve entities from a mesh, such as nodal
/// coordinates.
struct EntityRetrieval : public Mesh
{
    EntityRetrieval(Mesh aMeshBase);

    /// @brief The nodal coordinates ordered as x0,y0,z0,x1,y1,z1. This includes all nodes in the mesh.
    /// @note For 2D, only x and y coordinates are included in the vector.
    [[nodiscard]] auto flattenedNodalCoordinates() const -> std::vector<double>;

    /// @brief All nodal coordinates in the mesh (including the fixed blocks).
    [[nodiscard]] auto nodalCoordinates() const -> std::vector<third_party_integration::common::Coordinate>;

    /// @brief Centroids of all the elements in the mesh (including the fixed blocks).
    [[nodiscard]] auto elementCentroids() const -> std::vector<third_party_integration::common::Coordinate>;

    /// @brief Nodal coordinates in the mesh only associated with the design domain (not fixed blocks).
    [[nodiscard]] auto designDomainNodalCoordinates() const -> std::vector<third_party_integration::common::Coordinate>;

    /// @brief Centroids of the elements in the design domain of the mesh.
    [[nodiscard]] auto designDomainElementCentroids() const -> std::vector<third_party_integration::common::Coordinate>;
};

}  // namespace plato::mesh

#endif
