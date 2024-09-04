#include "plato/mesh/MeshDesignVariablesSequentialView.hpp"

#include <algorithm>
#include <cassert>
#include <numeric>

#include "plato/mesh/MeshDesignVariables.hpp"

namespace plato::mesh
{
template <typename MeshDesignVariablesType>
std::size_t MeshDesignVariablesSequentialViewTemplate<MeshDesignVariablesType>::size() const
{
    return std::distance(begin(), end());
}

template <typename MeshDesignVariablesType>
auto MeshDesignVariablesSequentialViewTemplate<MeshDesignVariablesType>::begin() const ->
    typename MeshDesignVariablesSequentialViewTemplate<MeshDesignVariablesType>::IteratorType
{
    using InnerIteratorType = typename IteratorType::InnerIterator;
    auto& tBlockScalarField = mMeshDesignVariables.get().mBlockScalarField;

    auto tBlockFieldBeginIterators = std::vector<InnerIteratorType>{};
    std::transform(tBlockScalarField.begin(), tBlockScalarField.end(), std::back_inserter(tBlockFieldBeginIterators),
                   [](auto& tBlockField) { return tBlockField.second.begin(); });

    auto tBlockFieldEndIterators = std::vector<InnerIteratorType>{};
    std::transform(tBlockScalarField.begin(), tBlockScalarField.end(), std::back_inserter(tBlockFieldEndIterators),
                   [](auto& tBlockField) { return tBlockField.second.end(); });

    return IteratorType{tBlockFieldBeginIterators, tBlockFieldEndIterators};
}

template <typename MeshDesignVariablesType>
auto MeshDesignVariablesSequentialViewTemplate<MeshDesignVariablesType>::end() const ->
    typename MeshDesignVariablesSequentialViewTemplate<MeshDesignVariablesType>::IteratorType
{
    using InnerIteratorType = typename IteratorType::InnerIterator;
    auto& tBlockScalarField = mMeshDesignVariables.get().mBlockScalarField;

    auto tBlockFieldEndIterators = std::vector<InnerIteratorType>{};
    std::transform(tBlockScalarField.begin(), tBlockScalarField.end(), std::back_inserter(tBlockFieldEndIterators),
                   [](auto& tBlockField) { return tBlockField.second.end(); });

    return IteratorType{tBlockFieldEndIterators, tBlockFieldEndIterators};
}

auto mesh_design_variables_to_vector(const MeshDesignVariablesSequentialView aMeshView) -> std::vector<ScalarFieldValue>
{
    auto tScalarField = std::vector<ScalarFieldValue>{};
    tScalarField.reserve(aMeshView.size());
    std::copy(aMeshView.begin(), aMeshView.end(), std::back_inserter(tScalarField));
    return tScalarField;
}

namespace detail
{
auto combine_scalar_field_values_and_ids(const std::vector<double>& aScalarField, const std::vector<std::size_t>& aIDs)
    -> std::vector<ScalarFieldValue>
{
    assert(aScalarField.size() == aIDs.size());
    auto tScalarField = std::vector<ScalarFieldValue>{};
    tScalarField.reserve(aScalarField.size());
    std::transform(aScalarField.cbegin(), aScalarField.cend(), aIDs.cbegin(), std::back_inserter(tScalarField),
                   [](const double aFieldValue, const ScalarFieldValue::IndexType aID) {
                       return ScalarFieldValue{aID, mesh::ScalarFieldValue::IndexType{0}, aFieldValue};
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

}  // namespace detail

// Explicit instantiations
template struct MeshDesignVariablesSequentialViewTemplate<MeshDesignVariables>;
template struct MeshDesignVariablesSequentialViewTemplate<const MeshDesignVariables>;
}  // namespace plato::mesh
