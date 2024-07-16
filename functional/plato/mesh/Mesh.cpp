#include "plato/mesh/Mesh.hpp"

#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/Utilities.hpp"
#include "plato/third_party_integration/stk_io/VolumeUtilities.hpp"

namespace plato::mesh
{
Mesh::Mesh(const std::filesystem::path& aMeshName)
    : mBulk(third_party_integration::stk_io::read_mesh_bulk_data(aMeshName))
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

unsigned int Mesh::numberOfBlocks() const
{
    assert(mBulk);
    return third_party_integration::stk_io::block_size(*mBulk);
}

std::optional<unsigned int> Mesh::blockId(const std::string_view aBlockName) const
{
    assert(mBulk);
    return third_party_integration::stk_io::block_id(*mBulk, aBlockName);
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

std::vector<third_party_integration::common::Coordinate> Mesh::nodalCoordinates() const
{
    assert(mBulk);
    return third_party_integration::stk_io::nodal_coordinates(*mBulk);
}

std::vector<third_party_integration::common::Coordinate> Mesh::elementCentroids() const
{
    assert(mBulk);
    return third_party_integration::stk_io::element_centroids(*mBulk);
}

double Mesh::volume() const
{
    assert(mBulk);
    return third_party_integration::stk_io::mesh_volume(*mBulk);
}

}  // namespace plato::mesh
