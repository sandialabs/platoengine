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
    const auto tBlockData = third_party_integration::stk_io::block_data(bulkData());
    std::vector<double> tElementVolumes;
    tElementVolumes.reserve(third_party_integration::stk_io::element_size(bulkData()));
    for (const auto& tBlock : tBlockData)
    {
        const auto& tPart = third_party_integration::stk_io::part_with_block_name(bulkData(), tBlock.mName);
        assert(tPart);
        const auto tElements = third_party_integration::stk_io::element_vector(bulkData(), tPart.value());
        std::transform(tElements.begin(), tElements.end(), std::back_inserter(tElementVolumes),
                       [&tBulkData = bulkData()](const auto& aElement)
                       { return third_party_integration::stk_io::element_volume(aElement, tBulkData); });
    }
    return tElementVolumes;
}

}  // namespace plato::mesh
