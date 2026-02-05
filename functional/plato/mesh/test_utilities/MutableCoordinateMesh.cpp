#include "plato/mesh/test_utilities/MutableCoordinateMesh.hpp"

#include "plato/third_party_integration/common/Vector3.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"

namespace plato::mesh::test_utilities
{
MutableCoordinateMesh::MutableCoordinateMesh(Mesh aMeshBase) : Mesh{std::move(aMeshBase)} {}

void MutableCoordinateMesh::updateNodalCoordinates(
    const std::vector<third_party_integration::common::Coordinate>& aNewCoordinates)
{
    third_party_integration::stk_io::replace_nodal_coordinate_values(bulkData(), aNewCoordinates);
}

void MutableCoordinateMesh::writeMeshToDisk(const std::filesystem::path& aMeshFilePath)
{
    third_party_integration::stk_io::write_bulk_data(aMeshFilePath, bulkData());
}
}  // namespace plato::mesh::test_utilities
