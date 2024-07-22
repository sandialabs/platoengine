#include "plato/mesh/DesignVariableConversion.hpp"

#include <algorithm>
#include <boost/numeric/conversion/cast.hpp>

#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/MeshBlocks.hpp"
#include "plato/utilities/IndexRange.hpp"

namespace plato::mesh
{
namespace
{
template <typename F>
MeshProxy entity_densities_to_mesh_proxy(const std::vector<double>& aDensities, const Mesh& aMesh, const F& aIDFunction)
{
    const auto tBlockData = MeshBlocks{aMesh}.blockData();

    auto tDensityIterator = aDensities.cbegin();
    auto tDensitiesBlockMap = MeshProxy::BlockDensities{};
    for (const auto& tBlock : tBlockData)
    {
        auto tDensitiesWithIndices = std::vector<Density>{};
        tDensitiesWithIndices.reserve(aDensities.size());
        const auto tEntityIDs = aIDFunction(aMesh, tBlock.mName);
        std::transform(tEntityIDs.cbegin(), tEntityIDs.cend(), tDensityIterator,
                       std::back_inserter(tDensitiesWithIndices),
                       [](const std::size_t aID, const double aDensity) {
                           return Density{aID, aDensity};
                       });
        tDensitiesBlockMap.emplace(tBlock.mID, std::move(tDensitiesWithIndices));
        std::advance(tDensityIterator, tEntityIDs.size());
    }
    return MeshProxy{aMesh.filePath(), std::move(tDensitiesBlockMap)};
}
}  // namespace

MeshProxy nodal_densities_to_mesh_proxy(const std::vector<double>& aDensities, const Mesh& aMesh)
{
    assert(EntityCounts{aMesh}.numberOfNodes() == aDensities.size());
    const auto tNodeIDs = [](const mesh::Mesh& aMesh, const std::string_view aBlockName)
    { return mesh::MeshBlocks{aMesh}.nodeIDs(aBlockName); };
    return entity_densities_to_mesh_proxy(aDensities, aMesh, tNodeIDs);
}

MeshProxy element_densities_to_mesh_proxy(const std::vector<double>& aDensities, const Mesh& aMesh)
{
    assert(EntityCounts{aMesh}.numberOfElements() == aDensities.size());
    const auto tNodeIDs = [](const mesh::Mesh& aMesh, const std::string_view aBlockName)
    { return mesh::MeshBlocks{aMesh}.elementIDs(aBlockName); };
    return entity_densities_to_mesh_proxy(aDensities, aMesh, tNodeIDs);
}

}  // namespace plato::mesh
