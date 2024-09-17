#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"

#include <algorithm>
#include <cassert>
#include <numeric>

#include "plato/analysis/AnalysisDomainMesh.hpp"

namespace plato::analysis
{
template <typename AnalysisDomainMeshType>
std::size_t AnalysisDomainMeshSequentialViewTemplate<AnalysisDomainMeshType>::size() const
{
    return std::distance(begin(), end());
}

template <typename AnalysisDomainMeshType>
auto AnalysisDomainMeshSequentialViewTemplate<AnalysisDomainMeshType>::begin() const ->
    typename AnalysisDomainMeshSequentialViewTemplate<AnalysisDomainMeshType>::IteratorType
{
    using InnerIteratorType = typename IteratorType::InnerIterator;
    auto& tBlockScalarField = mAnalysisDomainMesh.get().mBlockScalarField;

    auto tBlockFieldBeginIterators = std::vector<InnerIteratorType>{};
    std::transform(tBlockScalarField.begin(), tBlockScalarField.end(), std::back_inserter(tBlockFieldBeginIterators),
                   [](auto& tBlockField) { return tBlockField.second.begin(); });

    auto tBlockFieldEndIterators = std::vector<InnerIteratorType>{};
    std::transform(tBlockScalarField.begin(), tBlockScalarField.end(), std::back_inserter(tBlockFieldEndIterators),
                   [](auto& tBlockField) { return tBlockField.second.end(); });

    return IteratorType{tBlockFieldBeginIterators, tBlockFieldEndIterators};
}

template <typename AnalysisDomainMeshType>
auto AnalysisDomainMeshSequentialViewTemplate<AnalysisDomainMeshType>::end() const ->
    typename AnalysisDomainMeshSequentialViewTemplate<AnalysisDomainMeshType>::IteratorType
{
    using InnerIteratorType = typename IteratorType::InnerIterator;
    auto& tBlockScalarField = mAnalysisDomainMesh.get().mBlockScalarField;

    auto tBlockFieldEndIterators = std::vector<InnerIteratorType>{};
    std::transform(tBlockScalarField.begin(), tBlockScalarField.end(), std::back_inserter(tBlockFieldEndIterators),
                   [](auto& tBlockField) { return tBlockField.second.end(); });

    return IteratorType{tBlockFieldEndIterators, tBlockFieldEndIterators};
}

auto mesh_analysis_to_vector(const AnalysisDomainMeshSequentialView aMeshView) -> std::vector<ScalarFieldValue>
{
    auto tScalarField = std::vector<ScalarFieldValue>{};
    tScalarField.reserve(aMeshView.size());
    std::copy(aMeshView.begin(), aMeshView.end(), std::back_inserter(tScalarField));
    return tScalarField;
}

auto combine_scalar_field_values_and_ids(const std::vector<double>& aScalarField, const std::vector<std::size_t>& aIDs)
    -> std::vector<ScalarFieldValue>
{
    assert(aScalarField.size() == aIDs.size());
    auto tScalarField = std::vector<ScalarFieldValue>{};
    tScalarField.reserve(aScalarField.size());
    std::transform(aScalarField.cbegin(), aScalarField.cend(), aIDs.cbegin(), std::back_inserter(tScalarField),
                   [](const double aFieldValue, const ScalarFieldValue::IndexType aID) {
                       return ScalarFieldValue{aID, analysis::ScalarFieldValue::IndexType{0}, aFieldValue};
                   });
    return tScalarField;
}

auto split_scalar_field_values(const std::vector<ScalarFieldValue>& aScalarField)
    -> std::pair<std::vector<double>, std::vector<ScalarFieldValue::IndexType>>
{
    auto tFieldValues = std::vector<double>{};
    tFieldValues.reserve(aScalarField.size());
    std::transform(aScalarField.begin(), aScalarField.end(), std::back_inserter(tFieldValues),
                   [](const auto& aFieldValue) { return aFieldValue.mValue; });

    auto tIndices = std::vector<ScalarFieldValue::IndexType>{};
    tIndices.reserve(aScalarField.size());
    std::transform(aScalarField.begin(), aScalarField.end(), std::back_inserter(tIndices),
                   [](const auto& aFieldValue) { return aFieldValue.mGlobalMeshEntityID; });

    return {std::move(tFieldValues), std::move(tIndices)};
}

// Explicit instantiations
template struct AnalysisDomainMeshSequentialViewTemplate<AnalysisDomainMesh>;
template struct AnalysisDomainMeshSequentialViewTemplate<const AnalysisDomainMesh>;
}  // namespace plato::analysis
