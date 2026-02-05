#include "plato/mesh/MeshQuantities.hpp"

#include <vector>

#include "plato/third_party_integration/common/Vector3.hpp"
#include "plato/third_party_integration/stk_io/BlockUtilities.hpp"
#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"
#include "plato/third_party_integration/stk_io/VolumeUtilities.hpp"
#include "plato/utilities/Exception.hpp"

namespace plato::mesh
{
namespace
{
std::vector<double> element_volumes(const Mesh::PartReferenceVector& aPartVector, const stk::mesh::BulkData& aBulkData)
{
    namespace tpi = third_party_integration;

    const auto tBlockData = tpi::stk_io::block_data(aBulkData);
    auto tElementVolumes = std::vector<double>{};
    tElementVolumes.reserve(tpi::stk_io::element_size(aBulkData, aPartVector));
    for (const auto& tPart : aPartVector)
    {
        const auto tElements = tpi::stk_io::element_vector(aBulkData, tPart.get());
        std::transform(tElements.begin(), tElements.end(), std::back_inserter(tElementVolumes),
                       [&aBulkData](const auto& aElement) { return tpi::stk_io::element_volume(aElement, aBulkData); });
    }
    return tElementVolumes;
}
}  // namespace

MeshQuantities::MeshQuantities(Mesh aMeshBase) : Mesh{std::move(aMeshBase)} {}

double MeshQuantities::volume() const { return third_party_integration::stk_io::mesh_volume(bulkData()); }

auto MeshQuantities::volumeNodalSensitivities(const std::set<std::string>& aBlockNames) const
    -> std::vector<third_party_integration::common::Vector3>
{
    return third_party_integration::stk_io::volume_nodal_sensitivities(bulkData(), specifiedDomainBlocks(aBlockNames));
}

double MeshQuantities::averageNodalDensity() const
{
    const auto tTotalNumberOfNodes = third_party_integration::stk_io::node_size(bulkData());
    return static_cast<double>(tTotalNumberOfNodes) / volume();
}

std::vector<double> MeshQuantities::fixedDomainElementVolumes() const
{
    return element_volumes(fixedDomainBlocks(), bulkData());
}

std::vector<double> MeshQuantities::designDomainElementVolumes() const
{
    return element_volumes(designDomainBlocks(), bulkData());
}

std::vector<double> MeshQuantities::specifiedDomainElementVolumes(const std::set<std::string>& aBlockNames) const
{
    return element_volumes(specifiedDomainBlocks(aBlockNames), bulkData());
}

}  // namespace plato::mesh
