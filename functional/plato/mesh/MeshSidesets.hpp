#ifndef PLATO_MESH_MESHSIDESETS
#define PLATO_MESH_MESHSIDESETS

#include <string_view>

#include "plato/mesh/Mesh.hpp"
#include "plato/third_party_integration/krino/SensitivityTriangle.hpp"

namespace plato::mesh
{
/// @brief A mixin class extending Mesh and providing operations on mesh sidesets.
struct MeshSidesets : public Mesh
{
    explicit MeshSidesets(Mesh aMeshBase);

    /// @brief Returns the triangles in a named sideset
    [[nodiscard]] auto sidesetTriangles(const std::string& aSidesetName) const
        -> std::vector<third_party_integration::krino::SensitivityTriangle>;
};
}  // namespace plato::mesh

#endif
