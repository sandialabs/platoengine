#include "plato/mesh/MeshSidesets.hpp"

#include "plato/third_party_integration/krino/TriangleUtilities.hpp"

namespace plato::mesh
{

MeshSidesets::MeshSidesets(Mesh aMeshBase) : Mesh{std::move(aMeshBase)} {}

auto MeshSidesets::sidesetTriangles(const std::string& aSidesetName) const
    -> std::vector<third_party_integration::krino::SensitivityTriangle>
{
    return third_party_integration::krino::detail::get_interface_triangles(bulkData(), aSidesetName,
                                                                           designDomainBlocks());
}

}  // namespace plato::mesh
