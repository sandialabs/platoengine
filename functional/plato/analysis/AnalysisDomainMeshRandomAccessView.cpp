#include "plato/analysis/AnalysisDomainMeshRandomAccessView.hpp"

#include <numeric>

#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"

namespace plato::analysis
{
namespace
{
template <typename T>
struct ScalarFieldValueFromMesh
{
};

template <>
struct ScalarFieldValueFromMesh<AnalysisDomainMesh&>
{
    using type = AnalysisDomainMesh::ScalarFieldVector::value_type;
    using iterator = AnalysisDomainMesh::ScalarFieldVector::iterator;
};

template <>
struct ScalarFieldValueFromMesh<const AnalysisDomainMesh&>
{
    using type = const AnalysisDomainMesh::ScalarFieldVector::value_type;
    using iterator = AnalysisDomainMesh::ScalarFieldVector::const_iterator;
};

template <typename AnalysisDomainMeshType>
using SharedProxyType = SharedValueProxy<ScalarFieldValue,
                                         typename ScalarFieldValueFromMesh<AnalysisDomainMeshType>::iterator,
                                         typename AnalysisDomainMeshMutableRandomAccessView::AssignDensity>;

template <typename AnalysisDomainMeshVectorType>
auto vector_element_with_global_index(AnalysisDomainMeshVectorType&& aScalarField,
                                      const ScalarFieldValue::IndexType aIndex)
{
    assert(std::is_sorted(aScalarField.begin(), aScalarField.end(),
                          [](const auto& aEntryLeft, const auto& aEntryRight)
                          { return aEntryLeft.mGlobalMeshEntityID < aEntryRight.mGlobalMeshEntityID; }));

    const auto tEntryToFind = ScalarFieldValue{aIndex, 0, 0.0};
    const auto tIter = std::lower_bound(aScalarField.begin(), aScalarField.end(), tEntryToFind,
                                        [](const auto& aEntryLeft, const auto& aEntryRight)
                                        { return aEntryLeft.mGlobalMeshEntityID < aEntryRight.mGlobalMeshEntityID; });
    using OptionalType = decltype(std::make_optional(tIter));
    if (tIter != aScalarField.cend() && tIter->mGlobalMeshEntityID == aIndex)
    {
        return OptionalType{tIter};
    }
    return OptionalType{};
}

auto analysis_domain_mesh_size(const AnalysisDomainMesh& aAnalysisDomainMesh) -> std::size_t
{
    return AnalysisDomainMeshSequentialView{aAnalysisDomainMesh}.size();
}

template <typename AnalysisDomainMeshType>
auto mesh_element_with_global_index(AnalysisDomainMeshType&& aAnalysisDomainMesh,
                                    const ScalarFieldValue::IndexType aIndex)
{
    auto tSharedValueProxy = SharedProxyType<AnalysisDomainMeshType>{};

    for (auto& tBlockScalarField : aAnalysisDomainMesh.mBlockScalarField)
    {
        if (const auto tResult = vector_element_with_global_index(tBlockScalarField.second, aIndex))
        {
            tSharedValueProxy.mIterators.push_back(tResult.value());
        }
    }
    return tSharedValueProxy;
}
}  // namespace

auto AnalysisDomainMeshRandomAccessView::size() const -> std::size_t
{
    return analysis_domain_mesh_size(mAnalysisDomainMesh.get());
}

auto AnalysisDomainMeshRandomAccessView::operator[](const ScalarFieldValue::IndexType aIndex) const
    -> std::optional<ScalarFieldValue>
{
    const auto tEntries = mesh_element_with_global_index(mAnalysisDomainMesh.get(), aIndex);
    if (tEntries.mIterators.empty())
    {
        return std::nullopt;
    }
    return *tEntries.mIterators.front();
}

auto AnalysisDomainMeshMutableRandomAccessView::size() const -> std::size_t
{
    return analysis_domain_mesh_size(mAnalysisDomainMesh.get());
}

auto AnalysisDomainMeshMutableRandomAccessView::operator[](const ScalarFieldValue::IndexType aIndex) const
    -> SharedValueProxy<ScalarFieldValue, AnalysisDomainMesh::ScalarFieldVector::iterator, AssignDensity>
{
    return mesh_element_with_global_index(mAnalysisDomainMesh.get(), aIndex);
}

}  // namespace plato::analysis
