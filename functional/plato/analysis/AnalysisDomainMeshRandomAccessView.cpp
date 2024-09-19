#include "plato/analysis/AnalysisDomainMeshRandomAccessView.hpp"

#include <numeric>

#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"

namespace plato::analysis
{
namespace
{
std::optional<ScalarFieldValue> element_with_global_index(const AnalysisDomainMesh::ScalarFieldVector& aScalarField,
                                                          const ScalarFieldValue::IndexType aIndex)
{
    const auto tEntryToFind = ScalarFieldValue{aIndex, 0, 0.0};
    const auto tIter = std::lower_bound(aScalarField.cbegin(), aScalarField.cend(), tEntryToFind,
                                        [](const auto& aEntryLeft, const auto& aEntryRight)
                                        { return aEntryLeft.mGlobalMeshEntityID < aEntryRight.mGlobalMeshEntityID; });
    if (tIter != aScalarField.cend() && tIter->mGlobalMeshEntityID == aIndex)
    {
        return *tIter;
    }
    return std::nullopt;
}
}  // namespace

auto AnalysisDomainMeshRandomAccessView::size() const -> std::size_t
{
    return AnalysisDomainMeshSequentialView{mAnalysisDomainMesh.get()}.size();
}

auto AnalysisDomainMeshRandomAccessView::operator[](const ScalarFieldValue::IndexType aIndex) const
    -> std::optional<ScalarFieldValue>
{
    for (const auto& tBlockScalarField : mAnalysisDomainMesh.get().mBlockScalarField)
    {
        if (const auto tResult = element_with_global_index(tBlockScalarField.second, aIndex))
        {
            return tResult;
        }
    }
    return std::nullopt;
}

}  // namespace plato::analysis
