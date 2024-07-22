#ifndef PLATO_MESH_ENTITYCOUNTS
#define PLATO_MESH_ENTITYCOUNTS

#include "plato/mesh/Mesh.hpp"

namespace plato::mesh
{
/// @brief A mixin class for Mesh that provides utilities counting entities in a mesh, such as nodes.
struct EntityCounts : public Mesh
{
    EntityCounts(Mesh aMeshBase);

    /// @brief The total number of elements in the mesh
    [[nodiscard]] unsigned int numberOfElements() const;

    /// @brief The total number of nodes in the mesh
    [[nodiscard]] unsigned int numberOfNodes() const;

    /// @brief The total number of blocks in the mesh
    [[nodiscard]] unsigned int numberOfBlocks() const;

    /// @brief The dimensions of the mesh (2 or 3).
    [[nodiscard]] unsigned int spatialDimensions() const;
};

}  // namespace plato::mesh

#endif
