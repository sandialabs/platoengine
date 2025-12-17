#include "plato/third_party_integration/krino/SensitivityMapUtilities.hpp"

#include <mpi.h>

#include <Akri_CDFEM_Support.hpp>     //CDFEM_Support
#include <Akri_ChildNodeStencil.hpp>  //ChildNodeStencil
#include <Akri_LevelSet.hpp>          //LevelSet
#include <Akri_LevelSetPolicy.hpp>    //LSPerInterfacePolicy
#include <Akri_LevelSetShapeSensitivities.hpp>
#include <algorithm>
#include <boost/mpi/collectives.hpp>
#include <boost/mpi/collectives/all_gather.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/vector.hpp>
#include <iterator>
#include <stk_mesh/base/Entity.hpp>
#include <stk_mesh/base/Types.hpp>
#include <stk_util/environment/EnvData.hpp>

#include "plato/third_party_integration/krino/Utilities.hpp"
#include "plato/utilities/NamedType.hpp"
#include "plato/utilities/ReduceUtilities.hpp"

namespace plato::third_party_integration::krino
{
auto get_krino_sensitivities(const stk::mesh::BulkData& aBulkData,
                             const std::vector<::krino::LS_Field>& aLevelSetFields)
    -> std::vector<::krino::LevelSetShapeSensitivity>
{
    constexpr auto tComputeClosestPointSensitivities = false;
    return ::krino::get_levelset_shape_sensitivities(aBulkData, aLevelSetFields, tComputeClosestPointSensitivities);
}

auto coordinate_level_set_sensitivity(const ::krino::LevelSetShapeSensitivity& aLevelSetSensitivity,
                                      const unsigned int aSpatialDimension) -> std::vector<common::Vector3>
{
    std::vector<common::Vector3> tSensitivityVector;
    std::transform(aLevelSetSensitivity.dCoordsdParentLevelSets.begin(),
                   aLevelSetSensitivity.dCoordsdParentLevelSets.end(), std::back_inserter(tSensitivityVector),
                   [aSpatialDimension](const auto& tSensitivity)
                   {
                       const auto tZComponent = aSpatialDimension == 3U ? tSensitivity[2] : 0.0;
                       return common::Vector3{tSensitivity[0], tSensitivity[1], tZComponent};
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
        utilities::concatenate_over_all_ranks_and_sort(tCutMeshIdsFromMapOnThisRank, tCommunicator);

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

        if (unsigned int tCount = std::distance(tFirst, tLast); tCount > 1)
        {
            tHistogram[*tFirst] = tCount;
        }
        tFirst = tLast;
    }

    return tHistogram;
}

}  // namespace detail
}  // namespace plato::third_party_integration::krino
