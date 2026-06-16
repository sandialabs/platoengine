#include "plato/utilities/CartesianProduct.hpp"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <numeric>
#include <utility>
#include <vector>

namespace plato::utilities
{
CartesianProduct::CartesianProduct(const std::vector<std::size_t>& aSizes) : mMaxSizes(aSizes) {}

auto CartesianProduct::begin() const -> CartesianProductIterator { return {mMaxSizes}; }

auto CartesianProduct::end() const -> CartesianProductIterator { return CartesianProductIterator::end(mMaxSizes); }

CartesianProductIterator::CartesianProductIterator(std::vector<std::size_t> aSizes)
    : mMaxSizes(std::move(aSizes)), mCurrentState(mMaxSizes.size(), 0)
{
}

auto CartesianProductIterator::end(const std::vector<std::size_t>& aSizes) -> CartesianProductIterator
{
    CartesianProductIterator tIterator(aSizes);
    const auto tProduct =
        std::accumulate(tIterator.mMaxSizes.begin(), tIterator.mMaxSizes.end(), std::size_t{1}, std::multiplies<>());
    tIterator.mCurrentLinearIndex = tProduct;
    return tIterator;
}

auto CartesianProductIterator::operator*() const -> reference { return mCurrentState; }

auto CartesianProductIterator::operator->() const -> pointer { return &mCurrentState; }

auto CartesianProductIterator::operator++() -> CartesianProductIterator&
{
    ++mCurrentLinearIndex;

    std::transform(mMaxSizes.begin(), mMaxSizes.end(), mCurrentState.begin(),
                   [tCurrentLinearIndex = mCurrentLinearIndex](const auto aCurrentDimensionSize) mutable
                   {
                       const auto tDimensionIndex = tCurrentLinearIndex % aCurrentDimensionSize;
                       tCurrentLinearIndex /= aCurrentDimensionSize;
                       return tDimensionIndex;
                   });

    return *this;
}

auto CartesianProductIterator::operator++(int) -> CartesianProductIterator
{
    CartesianProductIterator tIterator = *this;
    ++(*this);
    return tIterator;
}

}  // namespace plato::utilities
