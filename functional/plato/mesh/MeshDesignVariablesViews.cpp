#include "plato/mesh/MeshDesignVariablesViews.hpp"

#include <algorithm>
#include <cassert>
#include <numeric>

#include "plato/mesh/MeshDesignVariables.hpp"
#include "plato/utilities/IndexRange.hpp"

namespace plato::mesh
{

template <typename OuterIteratorType, typename InnerIteratorType, typename IteratorCategory>
auto MeshDesignVariablesDensitiesViewIterator<OuterIteratorType, InnerIteratorType, IteratorCategory>::operator++()
    -> MeshDesignVariablesDensitiesViewIterator<OuterIteratorType, InnerIteratorType, IteratorCategory>&
{
    if (mInnerIterator)
    {
        ++mInnerIterator.value();
    }
    if (mInnerIterator == mOuterIterator->second.end())
    {
        ++mOuterIterator;
        mInnerIterator = innerIteratorBegin();
    }
    return *this;
}

template <typename OuterIteratorType, typename InnerIteratorType, typename IteratorCategory>
auto MeshDesignVariablesDensitiesViewIterator<OuterIteratorType, InnerIteratorType, IteratorCategory>::operator*() const
    -> const
    typename MeshDesignVariablesDensitiesViewIterator<OuterIteratorType, InnerIteratorType, IteratorCategory>::reference
{
    assert(mInnerIterator);
    return *mInnerIterator.value();
}

template <typename OuterIteratorType, typename InnerIteratorType, typename IteratorCategory>
bool MeshDesignVariablesDensitiesViewIterator<OuterIteratorType, InnerIteratorType, IteratorCategory>::operator==(
    const MeshDesignVariablesDensitiesViewIterator<OuterIteratorType, InnerIteratorType, IteratorCategory>&
        aRHSIterator) const
{
    return mOuterIterator == aRHSIterator.mOuterIterator && mInnerIterator == aRHSIterator.mInnerIterator;
}

template <typename OuterIteratorType, typename InnerIteratorType, typename IteratorCategory>
bool MeshDesignVariablesDensitiesViewIterator<OuterIteratorType, InnerIteratorType, IteratorCategory>::operator!=(
    const MeshDesignVariablesDensitiesViewIterator<OuterIteratorType, InnerIteratorType, IteratorCategory>&
        aRHSIterator) const
{
    return !(*this == aRHSIterator);
}

template <typename OuterIteratorType, typename InnerIteratorType, typename IteratorCategory>
auto MeshDesignVariablesDensitiesViewIterator<OuterIteratorType, InnerIteratorType, IteratorCategory>::
    innerIteratorBegin() const -> std::optional<InnerIteratorType>
{
    if (mOuterIterator != mOuterIteratorEnd)
    {
        return mOuterIterator->second.begin();
    }
    else
    {
        return std::nullopt;
    }
}

template <typename MeshDesignVariablesType>
std::size_t MeshDesignVariablesDensitiesViewTemplate<MeshDesignVariablesType>::size() const
{
    return std::accumulate(mMeshDesignVariables.get().mBlockDensities.begin(),
                           mMeshDesignVariables.get().mBlockDensities.end(), 0u,
                           [](const auto aSize, const auto& aBlock) { return aSize + aBlock.second.size(); });
}

template <typename MeshDesignVariablesType>
auto MeshDesignVariablesDensitiesViewTemplate<MeshDesignVariablesType>::begin() const ->
    typename MeshDesignVariablesDensitiesViewTemplate<MeshDesignVariablesType>::IteratorType
{
    using InnerIteratorType = typename IteratorType::InnerIterator;
    auto& tBlockDensities = mMeshDesignVariables.get().mBlockDensities;
    const auto tInnerIterator = tBlockDensities.begin() == tBlockDensities.end()
                                    ? std::optional<InnerIteratorType>{}
                                    : tBlockDensities.begin()->second.begin();
    return IteratorType{tBlockDensities.begin(), tBlockDensities.end(), tInnerIterator};
}

template <typename MeshDesignVariablesType>
auto MeshDesignVariablesDensitiesViewTemplate<MeshDesignVariablesType>::end() const ->
    typename MeshDesignVariablesDensitiesViewTemplate<MeshDesignVariablesType>::IteratorType
{
    auto& tBlockDensities = mMeshDesignVariables.get().mBlockDensities;
    return IteratorType{tBlockDensities.end(), tBlockDensities.end(), std::nullopt};
}

std::vector<Density> mesh_design_variables_to_vector(const MeshDesignVariablesDensitiesView aMeshView)
{
    auto tDensities = std::vector<Density>{};
    tDensities.reserve(aMeshView.size());
    std::copy(aMeshView.begin(), aMeshView.end(), std::back_inserter(tDensities));
    return tDensities;
}

auto combine_densities_and_ids(const std::vector<double>& aDensityValues, const std::vector<std::size_t>& aIDs)
    -> std::vector<Density>
{
    assert(aDensityValues.size() == aIDs.size());
    auto tDensities = std::vector<Density>{};
    tDensities.reserve(aDensityValues.size());
    std::transform(aDensityValues.cbegin(), aDensityValues.cend(), aIDs.cbegin(), std::back_inserter(tDensities),
                   [](const double aDensityValue, const Density::IndexType aID) {
                       return Density{aID, aDensityValue};
                   });
    return tDensities;
}

auto split_densities(const std::vector<Density>& aDensities)
    -> std::pair<std::vector<double>, std::vector<Density::IndexType>>
{
    auto tDensityValues = std::vector<double>{};
    tDensityValues.reserve(aDensities.size());
    std::transform(aDensities.begin(), aDensities.end(), std::back_inserter(tDensityValues),
                   [](const auto& aDensity) { return aDensity.mDensity; });

    auto tIndices = std::vector<Density::IndexType>{};
    tIndices.reserve(aDensities.size());
    std::transform(aDensities.begin(), aDensities.end(), std::back_inserter(tIndices),
                   [](const auto& aDensity) { return aDensity.mGlobalID; });

    return {std::move(tDensityValues), std::move(tIndices)};
}

// Explicit instantiations
template struct MeshDesignVariablesDensitiesViewTemplate<MeshDesignVariables>;
template struct MeshDesignVariablesDensitiesViewTemplate<const MeshDesignVariables>;
template struct MeshDesignVariablesDensitiesViewIterator<MeshDesignVariables::BlockDensities::iterator,
                                                         MeshDesignVariables::DensityVector::iterator,
                                                         std::output_iterator_tag>;
template struct MeshDesignVariablesDensitiesViewIterator<MeshDesignVariables::BlockDensities::const_iterator,
                                                         MeshDesignVariables::DensityVector::const_iterator,
                                                         std::forward_iterator_tag>;
}  // namespace plato::mesh
