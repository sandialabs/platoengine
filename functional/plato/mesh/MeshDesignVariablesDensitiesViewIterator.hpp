#ifndef PLATO_MESH_MESHDESIGNVARIABLESDENSITIESVIEWITERATOR
#define PLATO_MESH_MESHDESIGNVARIABLESDENSITIESVIEWITERATOR

#include <optional>
#include <type_traits>
#include <vector>

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

/// @brief Returns a SharedDensityProxy corresponding to the iterator in @a aCurrentIterators with the
///  smallest global id.
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
