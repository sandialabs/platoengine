#include "plato/mesh/MeshDesignVariablesDensitiesView.hpp"

#include <algorithm>
#include <cassert>
#include <numeric>

#include "plato/mesh/MeshDesignVariables.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::mesh
{
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
}  // namespace plato::mesh
