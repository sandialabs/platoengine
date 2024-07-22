#include "plato/mesh/EntityRetrieval.hpp"

#include "plato/third_party_integration/stk_io/Utilities.hpp"
#include "plato/third_party_integration/stk_io/VolumeUtilities.hpp"

namespace plato::mesh
{
EntityRetrieval::EntityRetrieval(Mesh aMeshBase) : Mesh{std::move(aMeshBase)} {}

std::vector<double> EntityRetrieval::flattenedNodalCoordinates() const
{
    return third_party_integration::stk_io::flattened_nodal_coordinates(bulkData());
}

std::vector<third_party_integration::common::Coordinate> EntityRetrieval::nodalCoordinates() const
{
    return third_party_integration::stk_io::nodal_coordinates(bulkData());
}

std::vector<third_party_integration::common::Coordinate> EntityRetrieval::elementCentroids() const
{
    return third_party_integration::stk_io::element_centroids(bulkData());
}
}  // namespace plato::mesh
