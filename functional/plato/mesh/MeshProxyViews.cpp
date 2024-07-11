#include "plato/mesh/MeshProxyViews.hpp"

#include <algorithm>

#include "plato/mesh/MeshProxy.hpp"

namespace plato::mesh
{
MeshProxyDensitiesViewIterator& MeshProxyDensitiesViewIterator::operator++()
{
    ++mIterator;
    return *this;
}

const double& MeshProxyDensitiesViewIterator::operator*() const { return *mIterator; }

bool MeshProxyDensitiesViewIterator::operator==(const MeshProxyDensitiesViewIterator& aRHSIterator) const
{
    return mIterator == aRHSIterator.mIterator;
}

bool MeshProxyDensitiesViewIterator::operator!=(const MeshProxyDensitiesViewIterator& aRHSIterator) const
{
    return !(*this == aRHSIterator);
}

double MeshProxyDensitiesView::operator[](unsigned aIndex) const { return mMeshProxy.get().mNodalDensities[aIndex]; }

std::size_t MeshProxyDensitiesView::size() const { return mMeshProxy.get().mNodalDensities.size(); }

MeshProxyDensitiesViewIterator MeshProxyDensitiesView::begin() const
{
    return MeshProxyDensitiesViewIterator{mMeshProxy.get().mNodalDensities.cbegin()};
}

MeshProxyDensitiesViewIterator MeshProxyDensitiesView::end() const
{
    return MeshProxyDensitiesViewIterator{mMeshProxy.get().mNodalDensities.cend()};
}

std::vector<double> to_vector(const MeshProxyDensitiesView aMeshView)
{
    auto tDensities = std::vector<double>{};
    tDensities.reserve(aMeshView.size());
    std::copy(aMeshView.begin(), aMeshView.end(), std::back_inserter(tDensities));
    return tDensities;
}

}  // namespace plato::mesh
