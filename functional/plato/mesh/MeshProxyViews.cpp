#include "plato/mesh/MeshProxyViews.hpp"

#include <algorithm>
#include <cassert>
#include <numeric>

#include "plato/mesh/MeshProxy.hpp"
#include "plato/utilities/IndexRange.hpp"

namespace plato::mesh
{

template <typename OuterIteratorType, typename InnerIteratorType, typename IteratorCategory>
auto MeshProxyDensitiesViewIterator<OuterIteratorType, InnerIteratorType, IteratorCategory>::operator++()
    -> MeshProxyDensitiesViewIterator<OuterIteratorType, InnerIteratorType, IteratorCategory>&
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
auto MeshProxyDensitiesViewIterator<OuterIteratorType, InnerIteratorType, IteratorCategory>::operator*() const -> const
    typename MeshProxyDensitiesViewIterator<OuterIteratorType, InnerIteratorType, IteratorCategory>::reference
{
    assert(mInnerIterator);
    return *mInnerIterator.value();
}

template <typename OuterIteratorType, typename InnerIteratorType, typename IteratorCategory>
bool MeshProxyDensitiesViewIterator<OuterIteratorType, InnerIteratorType, IteratorCategory>::operator==(
    const MeshProxyDensitiesViewIterator<OuterIteratorType, InnerIteratorType, IteratorCategory>& aRHSIterator) const
{
    return mOuterIterator == aRHSIterator.mOuterIterator && mInnerIterator == aRHSIterator.mInnerIterator;
}

template <typename OuterIteratorType, typename InnerIteratorType, typename IteratorCategory>
bool MeshProxyDensitiesViewIterator<OuterIteratorType, InnerIteratorType, IteratorCategory>::operator!=(
    const MeshProxyDensitiesViewIterator<OuterIteratorType, InnerIteratorType, IteratorCategory>& aRHSIterator) const
{
    return !(*this == aRHSIterator);
}

template <typename OuterIteratorType, typename InnerIteratorType, typename IteratorCategory>
auto MeshProxyDensitiesViewIterator<OuterIteratorType, InnerIteratorType, IteratorCategory>::innerIteratorBegin() const
    -> std::optional<InnerIteratorType>
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

template <typename MeshProxyType>
std::size_t MeshProxyDensitiesViewTemplate<MeshProxyType>::size() const
{
    return std::accumulate(mMeshProxy.get().mBlockDensities.begin(), mMeshProxy.get().mBlockDensities.end(), 0u,
                           [](const auto aSize, const auto& aBlock) { return aSize + aBlock.second.size(); });
}

template <typename MeshProxyType>
auto MeshProxyDensitiesViewTemplate<MeshProxyType>::begin() const ->
    typename MeshProxyDensitiesViewTemplate<MeshProxyType>::IteratorType
{
    using InnerIteratorType = typename IteratorType::InnerIterator;
    auto& tBlockDensities = mMeshProxy.get().mBlockDensities;
    const auto tInnerIterator = tBlockDensities.begin() == tBlockDensities.end()
                                    ? std::optional<InnerIteratorType>{}
                                    : tBlockDensities.begin()->second.begin();
    return IteratorType{tBlockDensities.begin(), tBlockDensities.end(), tInnerIterator};
}

template <typename MeshProxyType>
auto MeshProxyDensitiesViewTemplate<MeshProxyType>::end() const ->
    typename MeshProxyDensitiesViewTemplate<MeshProxyType>::IteratorType
{
    auto& tBlockDensities = mMeshProxy.get().mBlockDensities;
    return IteratorType{tBlockDensities.end(), tBlockDensities.end(), std::nullopt};
}

std::vector<Density> mesh_proxy_to_vector(const MeshProxyDensitiesView aMeshView)
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
template struct MeshProxyDensitiesViewTemplate<MeshProxy>;
template struct MeshProxyDensitiesViewTemplate<const MeshProxy>;
template struct MeshProxyDensitiesViewIterator<MeshProxy::BlockDensities::iterator,
                                               MeshProxy::DensityVector::iterator,
                                               std::output_iterator_tag>;
template struct MeshProxyDensitiesViewIterator<MeshProxy::BlockDensities::const_iterator,
                                               MeshProxy::DensityVector::const_iterator,
                                               std::forward_iterator_tag>;
}  // namespace plato::mesh
