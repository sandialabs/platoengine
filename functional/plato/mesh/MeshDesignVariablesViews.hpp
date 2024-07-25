#ifndef PLATO_MESH_MESHDESIGNVARIABLEVIEWS
#define PLATO_MESH_MESHDESIGNVARIABLEVIEWS

#include <functional>
#include <optional>

#include "plato/mesh/MeshDesignVariables.hpp"

namespace plato::mesh
{
template <typename IteratorType>
constexpr static bool kIsConstIterator =
    std::is_const_v<std::remove_reference_t<typename std::iterator_traits<IteratorType>::reference>>;

/// @brief An iterator type for using MeshDesignVariablesDensitiesView in std algorithms.
template <typename OuterIteratorType, typename InnerIteratorType, typename IteratorCategory>
struct MeshDesignVariablesDensitiesViewIterator
{
    using OuterIterator = OuterIteratorType;
    using InnerIterator = InnerIteratorType;

    using value_type = typename std::iterator_traits<InnerIteratorType>::value_type;
    using iterator_category = IteratorCategory;
    using difference_type = typename std::iterator_traits<InnerIteratorType>::difference_type;
    using pointer = typename std::iterator_traits<InnerIteratorType>::pointer;
    using reference = typename std::iterator_traits<InnerIteratorType>::reference;

    MeshDesignVariablesDensitiesViewIterator& operator++();
    [[nodiscard]] const reference operator*() const;

    template <typename Iterator = OuterIteratorType>
    [[nodiscard]] auto operator*() -> std::enable_if_t<!kIsConstIterator<Iterator>, reference>;

    [[nodiscard]] bool operator==(const MeshDesignVariablesDensitiesViewIterator& aRHSIterator) const;
    [[nodiscard]] bool operator!=(const MeshDesignVariablesDensitiesViewIterator& aRHSIterator) const;

    [[nodiscard]] auto innerIteratorBegin() const -> std::optional<InnerIteratorType>;

    OuterIteratorType mOuterIterator;
    OuterIteratorType mOuterIteratorEnd;
    std::optional<InnerIteratorType> mInnerIterator;
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
    using type = MeshDesignVariablesDensitiesViewIterator<MeshDesignVariables::BlockDensities::iterator,
                                                          MeshDesignVariables::DensityVector::iterator,
                                                          std::output_iterator_tag>;
};

template <>
struct IteratorType<const MeshDesignVariables>
{
    using type = MeshDesignVariablesDensitiesViewIterator<MeshDesignVariables::BlockDensities::const_iterator,
                                                          MeshDesignVariables::DensityVector::const_iterator,
                                                          std::forward_iterator_tag>;
};
}  // namespace detail

/// @brief The purpose of this object is to provide an interface for the density field in MeshDesignVariables.
///
/// Its main use is for facilitating copying density data from MeshDesignVariables to some other data structure,
/// such as that used by an external physics app.
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
std::vector<Density> mesh_design_variables_to_vector(MeshDesignVariablesDensitiesView aMeshView);

/// @brief Combines a vector of density values with a vector of global IDs into a single vector containing Density
/// objects.
auto combine_densities_and_ids(const std::vector<double>& aDensityValues, const std::vector<std::size_t>& aIDs)
    -> std::vector<Density>;

/// @brief Splits a vector of Density objects into the density values and node/element map.
auto split_densities(const std::vector<Density>& aDensities)
    -> std::pair<std::vector<double>, std::vector<std::size_t>>;

template <typename OuterIteratorType, typename InnerIteratorType, typename IteratorCategory>
template <typename Iterator>
auto MeshDesignVariablesDensitiesViewIterator<OuterIteratorType, InnerIteratorType, IteratorCategory>::operator*()
    -> std::enable_if_t<!kIsConstIterator<Iterator>, reference>
{
    assert(mInnerIterator.has_value());
    return *mInnerIterator.value();
}

}  // namespace plato::mesh

#endif
