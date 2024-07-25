#include "plato/mesh/MeshQuantities.hpp"

#include "plato/third_party_integration/stk_io/BlockUtilities.hpp"
#include "plato/third_party_integration/stk_io/Utilities.hpp"
#include "plato/third_party_integration/stk_io/VolumeUtilities.hpp"
#include "plato/utilities/PairWiseAccumulate.hpp"

namespace plato::mesh
{
MeshQuantities::MeshQuantities(Mesh aMeshBase) : Mesh{std::move(aMeshBase)} {}

double MeshQuantities::volume() const { return third_party_integration::stk_io::mesh_volume(bulkData()); }

double MeshQuantities::averageNodalDensity() const
{
    const auto tTotalNumberOfNodes = third_party_integration::stk_io::node_size(bulkData());
    return static_cast<double>(tTotalNumberOfNodes) / volume();
}

std::vector<double> MeshQuantities::elementVolumes() const
{
    namespace tpi = third_party_integration;

    const auto tBlockData = tpi::stk_io::block_data(bulkData());
    auto tElementVolumes = std::vector<double>{};
    tElementVolumes.reserve(tpi::stk_io::element_size(bulkData()));
    for (const auto& tBlock : tBlockData)
    {
        const auto& tPart = tpi::stk_io::part_with_block_meta_data_ordinal(bulkData(), tBlock.mMetaDataOrdinal);
        assert(tPart);
        const auto tElements = tpi::stk_io::element_vector(bulkData(), tPart.value());
        std::transform(tElements.begin(), tElements.end(), std::back_inserter(tElementVolumes),
                       [&tBulkData = bulkData()](const auto& aElement)
                       { return tpi::stk_io::element_volume(aElement, tBulkData); });
    }
    return tElementVolumes;
}

}  // namespace plato::mesh
