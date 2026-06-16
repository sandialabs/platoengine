#include "plato/third_party_integration/krino/SensitivityMapUtilities.hpp"

#include <Akri_LevelSetShapeSensitivities.hpp>
#include <algorithm>
#include <boost/mpi/collectives.hpp>
#include <cassert>
#include <iterator>
#include <stk_mesh/base/Types.hpp>
#include <unordered_map>
#include <vector>

#include "plato/third_party_integration/common/Vector3.hpp"
#include "plato/third_party_integration/krino/BackgroundMesh.hpp"
#include "plato/third_party_integration/krino/Utilities.hpp"
#include "plato/utilities/ReduceUtilities.hpp"

namespace plato::third_party_integration::krino
{
auto get_krino_sensitivities(const BackgroundMesh& aBackgroundMesh) -> std::vector<::krino::LevelSetShapeSensitivity>
{
    return ::krino::get_levelset_shape_sensitivities(aBackgroundMesh.mKrinoMesh->bulk_data(),
                                                     aBackgroundMesh.mLevelSetFields, kDoNormalSensitivities);
}

auto coordinate_level_set_sensitivity(const ::krino::LevelSetShapeSensitivity& aLevelSetSensitivity,
                                      const unsigned int aSpatialDimension) -> std::vector<common::Vector3>
{
    std::vector<common::Vector3> tSensitivityVector;
    std::transform(aLevelSetSensitivity.dCoordsdParentLevelSets.begin(),
                   aLevelSetSensitivity.dCoordsdParentLevelSets.end(), std::back_inserter(tSensitivityVector),
                   [aSpatialDimension](const auto& aSensitivity)
                   {
                       const auto tZComponent = aSpatialDimension == 3U ? aSensitivity[2] : 0.0;
                       return common::Vector3{aSensitivity[0], aSensitivity[1], tZComponent};
                   });
    return tSensitivityVector;
}

auto cut_mesh_node_id_multiplicity(const SensitivityMap& aSensitivityMap)
    -> std::unordered_map<CutMeshSurfaceNodeId, unsigned int>
{
    std::vector<CutMeshSurfaceNodeId> tCutMeshIdsFromMapOnThisRank;
    tCutMeshIdsFromMapOnThisRank.reserve(aSensitivityMap.size());
    std::transform(aSensitivityMap.begin(), aSensitivityMap.end(), std::back_inserter(tCutMeshIdsFromMapOnThisRank),
                   [](const auto aMapEntry) { return aMapEntry.first; });

    const auto tCommunicator = retrieve_mpi_communicator_from_krino();

    const auto tMergedSortedGlobalCutMeshIds =
        utilities::merge_on_all_ranks(tCutMeshIdsFromMapOnThisRank, tCommunicator);

    return utilities::compute_on_root<std::unordered_map<stk::mesh::EntityId, unsigned int>>(
        tCommunicator,
        [&tMergedSortedGlobalCutMeshIds]() { return detail::compute_histogram(tMergedSortedGlobalCutMeshIds); });
}

namespace detail
{

auto compute_histogram(const std::vector<stk::mesh::EntityId>& aGatheredSortedCutMeshNodeIDs)
    -> std::unordered_map<stk::mesh::EntityId, unsigned int>
{
    assert(std::is_sorted(aGatheredSortedCutMeshNodeIDs.begin(), aGatheredSortedCutMeshNodeIDs.end()));
    auto tHistogram = std::unordered_map<stk::mesh::EntityId, unsigned int>{};
    tHistogram.reserve(aGatheredSortedCutMeshNodeIDs.size());
    auto tFirst = aGatheredSortedCutMeshNodeIDs.begin();
    while (tFirst != aGatheredSortedCutMeshNodeIDs.end())
    {
        const auto tLast = std::find_if(tFirst, aGatheredSortedCutMeshNodeIDs.end(),
                                        [tFirst](const unsigned int aID) { return aID != *tFirst; });

        if (const auto tCount = std::distance(tFirst, tLast); tCount > 1U)
        {
            tHistogram[*tFirst] = tCount;
        }
        tFirst = tLast;
    }

    return tHistogram;
}

}  // namespace detail
}  // namespace plato::third_party_integration::krino
