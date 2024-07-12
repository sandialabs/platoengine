#include "plato/mesh/MeshProxyViews.hpp"

#include <algorithm>

#include "plato/mesh/MeshProxy.hpp"

namespace plato::mesh
{

template <typename IteratorType, typename IteratorCategory>
auto MeshProxyDensitiesViewIterator<IteratorType, IteratorCategory>::operator++()
    -> MeshProxyDensitiesViewIterator<IteratorType, IteratorCategory>&
{
    ++mIterator;
    return *this;
}

template <typename IteratorType, typename IteratorCategory>
const double& MeshProxyDensitiesViewIterator<IteratorType, IteratorCategory>::operator*() const
{
    return *mIterator;
}

template <typename IteratorType, typename IteratorCategory>
bool MeshProxyDensitiesViewIterator<IteratorType, IteratorCategory>::operator==(
    const MeshProxyDensitiesViewIterator<IteratorType, IteratorCategory>& aRHSIterator) const
{
    return mIterator == aRHSIterator.mIterator;
}

template <typename IteratorType, typename IteratorCategory>
bool MeshProxyDensitiesViewIterator<IteratorType, IteratorCategory>::operator!=(
    const MeshProxyDensitiesViewIterator<IteratorType, IteratorCategory>& aRHSIterator) const
{
    return !(*this == aRHSIterator);
}

template <typename MeshProxyType>
double MeshProxyDensitiesViewTemplate<MeshProxyType>::operator[](const unsigned aIndex) const
{
    return mMeshProxy.get().mNodalDensities[aIndex];
}

template <typename MeshProxyType>
std::size_t MeshProxyDensitiesViewTemplate<MeshProxyType>::size() const
{
    return mMeshProxy.get().mNodalDensities.size();
}

template <typename MeshProxyType>
auto MeshProxyDensitiesViewTemplate<MeshProxyType>::begin() const ->
    typename MeshProxyDensitiesViewTemplate<MeshProxyType>::IteratorType
{
    return IteratorType{mMeshProxy.get().mNodalDensities.begin()};
}

template <typename MeshProxyType>
auto MeshProxyDensitiesViewTemplate<MeshProxyType>::end() const ->
    typename MeshProxyDensitiesViewTemplate<MeshProxyType>::IteratorType
{
    return IteratorType{mMeshProxy.get().mNodalDensities.end()};
}

std::vector<double> to_vector(const MeshProxyDensitiesView aMeshView)
{
    auto tDensities = std::vector<double>{};
    tDensities.reserve(aMeshView.size());
    std::copy(aMeshView.begin(), aMeshView.end(), std::back_inserter(tDensities));
    return tDensities;
}

// Explicit instantiations
template struct MeshProxyDensitiesViewTemplate<MeshProxy>;
template struct MeshProxyDensitiesViewTemplate<const MeshProxy>;
template struct MeshProxyDensitiesViewIterator<std::vector<double>::iterator, std::output_iterator_tag>;
template struct MeshProxyDensitiesViewIterator<std::vector<double>::const_iterator, std::forward_iterator_tag>;
}  // namespace plato::mesh
