#include "plato/mesh/Mesh.hpp"

namespace plato::mesh
{
Mesh::Mesh(const std::filesystem::path& aMeshName)
    : mBulk(third_party_integration::stk_io::read_mesh_bulk_data(aMeshName))
{
}

Mesh::Mesh(const third_party_integration::stk_io::CommandGenerator& aCommandGenerator)
    : mBulk(third_party_integration::stk_io::generate_bulk_data(aCommandGenerator))
{
}

unsigned int Mesh::numberOfElements() const
{
    assert(mBulk);
    return third_party_integration::stk_io::element_size(*mBulk);
}

unsigned int Mesh::numberOfNodes() const
{
    assert(mBulk);
    return third_party_integration::stk_io::node_size(*mBulk);
}

unsigned int Mesh::spatialDimensions() const
{
    assert(mBulk);
    return third_party_integration::stk_io::spatial_dimensions(*mBulk);
}

std::vector<double> Mesh::flattenedNodalCoordinates() const
{
    assert(mBulk);
    return third_party_integration::stk_io::flattened_nodal_coordinates(*mBulk);
}

void Mesh::write_mesh(const std::filesystem::path& aOutputFileName) const
{
    third_party_integration::stk_io::write_bulk_data(aOutputFileName, mBulk);
}

}  // namespace plato::mesh
