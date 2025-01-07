#include "plato/analysis/Utilities.hpp"

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"
#include "plato/utilities/Enumerate.hpp"

namespace plato::analysis
{
namespace
{
template <typename AssociativeContainer, typename Predicate>
auto remove_entries_matching_predicate(AssociativeContainer&& aAssociativeContainer, const Predicate& aPredicate)
    -> AssociativeContainer
{
    for (auto tBlockIter = aAssociativeContainer.begin(); tBlockIter != aAssociativeContainer.end();)
    {
        if (aPredicate(tBlockIter->first))
        {
            tBlockIter = aAssociativeContainer.erase(tBlockIter);
        }
        else
        {
            ++tBlockIter;
        }
    }
    return std::forward<AssociativeContainer>(aAssociativeContainer);
}

auto renumber_vector_entries(AnalysisDomainMesh&& aAnalysisDomainMesh) -> AnalysisDomainMesh
{
    for (auto [tIndex, tScalarFieldValue] :
         utilities::enumerate(AnalysisDomainMeshMutableSequentialView{aAnalysisDomainMesh}))
    {
        const auto& tFieldValue = static_cast<analysis::ScalarFieldValue>(tScalarFieldValue);
        tScalarFieldValue = analysis::ScalarFieldValue{tFieldValue.mGlobalMeshEntityID, tIndex, tFieldValue.mValue};
    }
    return aAnalysisDomainMesh;
}
}  // namespace

auto zero_scalar_field(AnalysisDomainMesh&& aAnalysisDomainMesh) -> AnalysisDomainMesh
{
    for (auto tScalarFieldValue : AnalysisDomainMeshMutableSequentialView{aAnalysisDomainMesh})
    {
        const auto& tFieldIndices = static_cast<analysis::ScalarFieldValue>(tScalarFieldValue);
        tScalarFieldValue = analysis::ScalarFieldValue{tFieldIndices.mGlobalMeshEntityID,
                                                       tFieldIndices.mDesignVariableVectorIndex, 0.0};
    }
    return aAnalysisDomainMesh;
}

auto remove_block_fields(AnalysisDomainMesh&& aAnalysisDomainMesh,
                         const std::vector<AnalysisDomainMesh::BlockIDType>& aBlockIDs) -> AnalysisDomainMesh
{
    const auto tShouldRemoveBlock = [&aBlockIDs](const AnalysisDomainMesh::BlockIDType aBlockIDToCheck)
    { return std::find(aBlockIDs.cbegin(), aBlockIDs.cend(), aBlockIDToCheck) != aBlockIDs.cend(); };
    auto tMeshName = aAnalysisDomainMesh.mFileName;
    auto tBlockScalarFields =
        remove_entries_matching_predicate(std::move(aAnalysisDomainMesh.mBlockScalarField), tShouldRemoveBlock);
    return renumber_vector_entries(AnalysisDomainMesh{std::move(tMeshName), std::move(tBlockScalarFields)});
}
}  // namespace plato::analysis
