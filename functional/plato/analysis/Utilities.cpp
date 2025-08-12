#include <algorithm>

#include "plato/analysis/Utilities.hpp"

#include <boost/functional/hash.hpp>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"
#include "plato/utilities/Enumerate.hpp"
#include "plato/utilities/HashUtilities.hpp"

namespace plato::analysis
{
namespace
{
template <typename AssociativeContainer, typename Predicate>
auto remove_entries_matching_predicate(AssociativeContainer&& aAssociativeContainer,
                                       const Predicate& aPredicate) -> AssociativeContainer
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

auto hash_value(const ScalarFieldValue& aScalarFieldValue) -> std::size_t
{
    auto tSeed = std::size_t{0U};
    boost::hash_combine(tSeed, aScalarFieldValue.mGlobalMeshEntityID);
    boost::hash_combine(tSeed, aScalarFieldValue.mDesignVariableVectorIndex);
    boost::hash_combine(tSeed, aScalarFieldValue.mValue);
    return tSeed;
}

auto hash_value(const AnalysisDomainMesh& aAnalysisDomainMesh) -> std::size_t
{
    auto tHashValue = hash_value(aAnalysisDomainMesh.mFileName);
    for (const auto& [tBlockID, tScalarField] : aAnalysisDomainMesh.mBlockScalarField)
    {
        boost::hash_combine(tHashValue, tBlockID);
        boost::hash_combine(tHashValue, utilities::hash_container(tScalarField));
    }
    return tHashValue;
}
}  // namespace plato::analysis
