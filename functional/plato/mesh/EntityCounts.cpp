#include "plato/mesh/EntityCounts.hpp"

#include "plato/third_party_integration/stk_io/BlockUtilities.hpp"
#include "plato/third_party_integration/stk_io/Utilities.hpp"

namespace plato::mesh
{
EntityCounts::EntityCounts(Mesh aMeshBase) : Mesh{std::move(aMeshBase)} {}

unsigned int EntityCounts::numberOfElements() const
{
    return third_party_integration::stk_io::element_size(bulkData());
}

unsigned int EntityCounts::numberOfNodes() const { return third_party_integration::stk_io::node_size(bulkData()); }

unsigned int EntityCounts::numberOfBlocks() const { return third_party_integration::stk_io::block_size(bulkData()); }

unsigned int EntityCounts::spatialDimensions() const
{
    return third_party_integration::stk_io::spatial_dimensions(bulkData());
}

}  // namespace plato::mesh
