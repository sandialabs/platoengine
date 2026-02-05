#ifndef PLATO_MESH_TESTUTILITIES_MUTABLECOORDINATEMESH
#define PLATO_MESH_TESTUTILITIES_MUTABLECOORDINATEMESH

#include "plato/mesh/Mesh.hpp"

namespace plato::third_party_integration::common
{
struct Coordinate;
}

/// @brief mixin extension for Mesh that provides a function for overwriting nodal coordinates. This is used for
/// checking the gradient of functions with respect to nodal coordinates.
namespace plato::mesh::test_utilities
{
struct MutableCoordinateMesh : public Mesh
{
    explicit MutableCoordinateMesh(Mesh aMeshBase);

    /// @brief overwrites nodal coordinate field with values in @a aNewCoordinates.
    /// @pre size of @a aNewCoordinates must be equal to the total number of nodes in the mesh.
    void updateNodalCoordinates(const std::vector<third_party_integration::common::Coordinate>& aNewCoordinates);

    /// @brief writes the mesh with new nodal coordinates to disp at the path @a aMeshFilePath.
    void writeMeshToDisk(const std::filesystem::path& aMeshFilePath);
};
}  // namespace plato::mesh::test_utilities

#endif
