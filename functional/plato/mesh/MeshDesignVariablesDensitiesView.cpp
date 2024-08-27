#include "plato/mesh/MeshDesignVariablesDensitiesView.hpp"

#include <algorithm>
#include <cassert>
#include <numeric>

#include "plato/mesh/MeshDesignVariables.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::mesh
{
template <typename InnerIteratorType, typename IteratorCategory>
auto MeshDesignVariablesDensitiesViewIterator<InnerIteratorType, IteratorCategory>::operator++()
    -> MeshDesignVariablesDensitiesViewIterator<InnerIteratorType, IteratorCategory>&
{
    const auto tValues = detail::dereference_all(mCurrentIterators, mEndIterators);
    const auto tMinIDIterator = detail::min_id_iterator(tValues);
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
auto MeshDesignVariablesDensitiesViewIterator<InnerIteratorType, IteratorCategory>::operator*() const -> const reference
{
    return detail::dereferenced_proxy(mCurrentIterators, mEndIterators);
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

auto min_id_iterator(const std::vector<std::optional<Density>>& aDensities) ->
    typename std::vector<std::optional<Density>>::const_iterator
{
    return std::min_element(aDensities.cbegin(), aDensities.cend(),
                            [](const auto& tLeftDensity, const auto& tRightDensity)
                            {
                                if (tLeftDensity && tRightDensity)
                                {
                                    return tLeftDensity->mGlobalMeshEntityID < tRightDensity->mGlobalMeshEntityID;
                                }
                                return tLeftDensity.has_value();
                            });
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
