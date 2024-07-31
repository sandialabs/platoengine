#include "plato/mesh/MeshDesignVariablesViews.hpp"

#include <algorithm>
#include <cassert>
#include <numeric>

#include "plato/mesh/MeshDesignVariables.hpp"
#include "plato/utilities/IndexRange.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::mesh
{
namespace
{
template <typename InnerIteratorType>
std::vector<std::optional<Density>> dereference_all(const std::vector<InnerIteratorType>& aCurrentIterators,
                                                    const std::vector<InnerIteratorType>& aEndIterators)
{
    auto tDensities = std::vector<std::optional<Density>>{};
    tDensities.reserve(tDensities.size());
    std::transform(aCurrentIterators.cbegin(), aCurrentIterators.cend(), aEndIterators.cbegin(),
                   std::back_inserter(tDensities),
                   [](const auto& aCurrentIterator, const auto& aEndIterator) -> std::optional<Density>
                   {
                       if (aCurrentIterator != aEndIterator)
                       {
                           return *aCurrentIterator;
                       }
                       return std::nullopt;
                   });
    return tDensities;
}
}  // namespace

template <typename InnerIteratorType, typename IteratorCategory>
auto MeshDesignVariablesDensitiesViewIterator<InnerIteratorType, IteratorCategory>::operator++()
    -> MeshDesignVariablesDensitiesViewIterator<InnerIteratorType, IteratorCategory>&
{
    const auto tValues = dereference_all(mCurrentIterators, mEndIterators);
    const auto tMinIDIterator =
        std::min_element(tValues.cbegin(), tValues.cend(),
                         [](const auto& tLeftDensity, const auto& tRightDensity)
                         {
                             if (tLeftDensity && tRightDensity)
                             {
                                 return tLeftDensity->mGlobalMeshEntityID < tRightDensity->mGlobalMeshEntityID;
                             }
                             return tLeftDensity.has_value();
                         });
    if (tMinIDIterator != tValues.cend() && tMinIDIterator->has_value())
    {
        const auto tMinGlobalID = tMinIDIterator->value().mGlobalMeshEntityID;  // NOLINT
        for (const auto& [tCurrentIterator, tEndIterator] : utilities::Zip{mCurrentIterators, mEndIterators})
        {
            if (tCurrentIterator != tEndIterator && tCurrentIterator->mGlobalMeshEntityID == tMinGlobalID)
            {
                ++tCurrentIterator;
            }
        }
    }
    return *this;
}

template <typename InnerIteratorType, typename IteratorCategory>
auto MeshDesignVariablesDensitiesViewIterator<InnerIteratorType, IteratorCategory>::operator*() const -> const
    typename MeshDesignVariablesDensitiesViewIterator<InnerIteratorType, IteratorCategory>::reference
{
    const auto tValues = dereference_all(mCurrentIterators, mEndIterators);
    const auto tMinIDIterator =
        std::min_element(tValues.cbegin(), tValues.cend(),
                         [](const auto& tLeftDensity, const auto& tRightDensity)
                         {
                             if (tLeftDensity && tRightDensity)
                             {
                                 return tLeftDensity->mGlobalMeshEntityID < tRightDensity->mGlobalMeshEntityID;
                             }
                             return tLeftDensity.has_value();
                         });
    if (!tMinIDIterator->has_value())
    {
        return SharedDensityProxy<InnerIteratorType>{};
    }
    const auto tMinGlobalID = tMinIDIterator->value().mGlobalMeshEntityID;  // NOLINT
    auto tProxy = SharedDensityProxy<InnerIteratorType>{};
    for (const auto& [tCurrentIterator, tEndIterator] : utilities::Zip{mCurrentIterators, mEndIterators})
    {
        if (tCurrentIterator != tEndIterator && tCurrentIterator->mGlobalMeshEntityID == tMinGlobalID)
        {
            tProxy.mIterators.push_back(tCurrentIterator);
        }
    }
    return tProxy;
}

template <typename InnerIteratorType, typename IteratorCategory>
bool MeshDesignVariablesDensitiesViewIterator<InnerIteratorType, IteratorCategory>::operator==(
    const MeshDesignVariablesDensitiesViewIterator<InnerIteratorType, IteratorCategory>& aRHSIterator) const
{
    auto tAllEqual = true;
    for (const auto& [tLeftIterator, tRightIterator] :
         utilities::Zip{mCurrentIterators, aRHSIterator.mCurrentIterators})
    {
        tAllEqual &= tLeftIterator == tRightIterator;
    }
    return tAllEqual;
}

template <typename InnerIteratorType, typename IteratorCategory>
bool MeshDesignVariablesDensitiesViewIterator<InnerIteratorType, IteratorCategory>::operator!=(
    const MeshDesignVariablesDensitiesViewIterator<InnerIteratorType, IteratorCategory>& aRHSIterator) const
{
    return !(*this == aRHSIterator);
}

template <typename MeshDesignVariablesType>
std::size_t MeshDesignVariablesDensitiesViewTemplate<MeshDesignVariablesType>::size() const
{
    auto tIter = begin();
    const auto tEnd = end();
    auto tCount = std::size_t{0};
    while (tIter != tEnd)
    {
        ++tIter;
        ++tCount;
    }
    return tCount;
}

template <typename MeshDesignVariablesType>
auto MeshDesignVariablesDensitiesViewTemplate<MeshDesignVariablesType>::begin() const ->
    typename MeshDesignVariablesDensitiesViewTemplate<MeshDesignVariablesType>::IteratorType
{
    using InnerIteratorType = typename IteratorType::InnerIterator;
    auto& tBlockDensities = mMeshDesignVariables.get().mBlockDensities;

    auto tBlockDensityBeginIterators = std::vector<InnerIteratorType>{};
    std::transform(tBlockDensities.begin(), tBlockDensities.end(), std::back_inserter(tBlockDensityBeginIterators),
                   [](auto& tBlockDensity) { return tBlockDensity.second.begin(); });

    auto tBlockDensityEndIterators = std::vector<InnerIteratorType>{};
    std::transform(tBlockDensities.begin(), tBlockDensities.end(), std::back_inserter(tBlockDensityEndIterators),
                   [](auto& tBlockDensity) { return tBlockDensity.second.end(); });

    return IteratorType{tBlockDensityBeginIterators, tBlockDensityEndIterators};
}

template <typename MeshDesignVariablesType>
auto MeshDesignVariablesDensitiesViewTemplate<MeshDesignVariablesType>::end() const ->
    typename MeshDesignVariablesDensitiesViewTemplate<MeshDesignVariablesType>::IteratorType
{
    using InnerIteratorType = typename IteratorType::InnerIterator;
    auto& tBlockDensities = mMeshDesignVariables.get().mBlockDensities;

    auto tBlockDensityEndIterators = std::vector<InnerIteratorType>{};
    std::transform(tBlockDensities.begin(), tBlockDensities.end(), std::back_inserter(tBlockDensityEndIterators),
                   [](auto& tBlockDensity) { return tBlockDensity.second.end(); });

    return IteratorType{tBlockDensityEndIterators, tBlockDensityEndIterators};
}

std::vector<Density> mesh_design_variables_to_vector(const MeshDesignVariablesDensitiesView aMeshView)
{
    auto tDensities = std::vector<Density>{};
    tDensities.reserve(aMeshView.size());
    std::copy(aMeshView.begin(), aMeshView.end(), std::back_inserter(tDensities));
    return tDensities;
}

namespace detail
{
auto combine_densities_and_ids(const std::vector<double>& aDensityValues, const std::vector<std::size_t>& aIDs)
    -> std::vector<Density>
{
    assert(aDensityValues.size() == aIDs.size());
    auto tDensities = std::vector<Density>{};
    tDensities.reserve(aDensityValues.size());
    std::transform(aDensityValues.cbegin(), aDensityValues.cend(), aIDs.cbegin(), std::back_inserter(tDensities),
                   [](const double aDensityValue, const Density::IndexType aID) {
                       return Density{aID, mesh::Density::IndexType{0}, aDensityValue};
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
                   [](const auto& aDensity) { return aDensity.mGlobalMeshEntityID; });

    return {std::move(tDensityValues), std::move(tIndices)};
}
}  // namespace detail

// Explicit instantiations
template struct MeshDesignVariablesDensitiesViewTemplate<MeshDesignVariables>;
template struct MeshDesignVariablesDensitiesViewTemplate<const MeshDesignVariables>;
template struct MeshDesignVariablesDensitiesViewIterator<MeshDesignVariables::DensityVector::iterator,
                                                         std::output_iterator_tag>;
template struct MeshDesignVariablesDensitiesViewIterator<MeshDesignVariables::DensityVector::const_iterator,
                                                         std::input_iterator_tag>;
}  // namespace plato::mesh
