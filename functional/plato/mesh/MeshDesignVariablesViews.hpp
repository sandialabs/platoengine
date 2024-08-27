#ifndef PLATO_MESH_MESHDESIGNVARIABLEVIEWS
#define PLATO_MESH_MESHDESIGNVARIABLEVIEWS

#include <cassert>
#include <functional>
#include <optional>

#include "plato/mesh/MeshDesignVariables.hpp"
#include "plato/mesh/SharedDensityProxy.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::mesh
{
/// @brief An iterator type for using MeshDesignVariablesDensitiesView in std algorithms.
template <typename InnerIteratorType, typename IteratorCategory>
struct MeshDesignVariablesDensitiesViewIterator
{
    using InnerIterator = InnerIteratorType;

    using value_type = typename std::iterator_traits<InnerIteratorType>::value_type;
    using iterator_category = IteratorCategory;
    using difference_type = typename std::iterator_traits<InnerIteratorType>::difference_type;
    using pointer = typename std::iterator_traits<InnerIteratorType>::pointer;
    using reference = SharedDensityProxy<InnerIteratorType>;

    MeshDesignVariablesDensitiesViewIterator& operator++();
    [[nodiscard]] const reference operator*() const;

    template <typename Iterator = InnerIteratorType>
    [[nodiscard]] auto operator*() -> std::enable_if_t<!kIsConstIterator<Iterator>, reference>;

    [[nodiscard]] bool operator==(const MeshDesignVariablesDensitiesViewIterator& aRHSIterator) const;
    [[nodiscard]] bool operator!=(const MeshDesignVariablesDensitiesViewIterator& aRHSIterator) const;

    std::vector<InnerIteratorType> mCurrentIterators;
    std::vector<InnerIteratorType> mEndIterators;
};

namespace detail
{
template <typename MeshDesignVariablesType>
struct IteratorType
{
};

template <>
struct IteratorType<MeshDesignVariables>
{
    using type = MeshDesignVariablesDensitiesViewIterator<MeshDesignVariables::DensityVector::iterator,
                                                          std::output_iterator_tag>;
};

template <>
struct IteratorType<const MeshDesignVariables>
{
    using type = MeshDesignVariablesDensitiesViewIterator<MeshDesignVariables::DensityVector::const_iterator,
                                                          std::input_iterator_tag>;
};
}  // namespace detail

/// @brief The purpose of this object is to provide an interface for the density field in MeshDesignVariables.
///
/// Its main use is for facilitating copying density data from MeshDesignVariables to some other data structure,
/// such as that used by an external physics app.
/// The iterator range it provides will iterate over the contained entity global IDs in ascending order, and only
/// visit each once, even if that entity is shared between blocks. For example, nodes may be shared between blocks
/// on any shared boundaries and so the same density value associated with a node may be in two or more blocks.
template <typename MeshDesignVariablesType>
struct MeshDesignVariablesDensitiesViewTemplate
{
    std::reference_wrapper<MeshDesignVariablesType> mMeshDesignVariables;

    [[nodiscard]] std::size_t size() const;

    using IteratorType = typename detail::IteratorType<MeshDesignVariablesType>::type;
    [[nodiscard]] IteratorType begin() const;
    [[nodiscard]] IteratorType end() const;
};

using MeshDesignVariablesDensitiesView = MeshDesignVariablesDensitiesViewTemplate<const MeshDesignVariables>;
using MeshDesignVariablesDensitiesMutableView = MeshDesignVariablesDensitiesViewTemplate<MeshDesignVariables>;

/// @brief Converts the densities associated with the mesh in @a aMeshView to a `std::vector`.
[[nodiscard]] std::vector<Density> mesh_design_variables_to_vector(MeshDesignVariablesDensitiesView aMeshView);

namespace detail
{
/// @brief Combines a vector of density values with a vector of global IDs into a single vector containing Density
/// objects.
/// @warning This does not set the field `mDesignVariableVectorIndex` in the returned values. This is meant for use as
/// an initial step to combine global IDs and density values.
[[nodiscard]] auto combine_densities_and_ids(const std::vector<double>& aDensityValues,
                                             const std::vector<std::size_t>& aIDs) -> std::vector<Density>;

/// @brief Splits a vector of Density objects into the density values and node/element map.
[[nodiscard]] auto split_densities(const std::vector<Density>& aDensities)
    -> std::pair<std::vector<double>, std::vector<std::size_t>>;

/// @brief Returns an iterator to the element with the smallest ID.
[[nodiscard]] auto min_id_iterator(const std::vector<std::optional<Density>>& aDensities) ->
    typename std::vector<std::optional<Density>>::const_iterator;

/// @brief Dereferences all iterators in @a aCurrentIterators if they are not equal to their corresponding end iterators
/// in @a aEndIterators. If an iterator is equal to its end iterator, an empty optional is used.
template <typename InnerIteratorType>
auto dereference_all(const std::vector<InnerIteratorType>& aCurrentIterators,
                     const std::vector<InnerIteratorType>& aEndIterators) -> std::vector<std::optional<Density>>
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

template <typename InnerIteratorType>
auto dereferenced_proxy(const std::vector<InnerIteratorType>& aCurrentIterators,
                        const std::vector<InnerIteratorType>& aEndIterators)
{
    const auto tValues = detail::dereference_all(aCurrentIterators, aEndIterators);
    const auto tMinIDIterator = detail::min_id_iterator(tValues);
    if (!tMinIDIterator->has_value())
    {
        return SharedDensityProxy<InnerIteratorType>{};
    }
    const auto tMinGlobalID = tMinIDIterator->value().mGlobalMeshEntityID;  // NOLINT
    auto tProxy = SharedDensityProxy<InnerIteratorType>{};
    for (const auto& [tCurrentIterator, tEndIterator] : utilities::Zip{aCurrentIterators, aEndIterators})
    {
        if (tCurrentIterator != tEndIterator && tCurrentIterator->mGlobalMeshEntityID == tMinGlobalID)
        {
            tProxy.mIterators.push_back(tCurrentIterator);
        }
    }
    return tProxy;
}
}  // namespace detail

template <typename InnerIteratorType, typename IteratorCategory>
template <typename Iterator>
auto MeshDesignVariablesDensitiesViewIterator<InnerIteratorType, IteratorCategory>::operator*()
    -> std::enable_if_t<!kIsConstIterator<Iterator>, reference>
{
    return detail::dereferenced_proxy(mCurrentIterators, mEndIterators);
}

}  // namespace plato::mesh

#endif
