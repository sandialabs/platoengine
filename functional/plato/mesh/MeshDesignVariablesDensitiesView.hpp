#ifndef PLATO_MESH_MESHDESIGNVARIABLESDENSITIESVIEW
#define PLATO_MESH_MESHDESIGNVARIABLESDENSITIESVIEW

#include <functional>
#include <optional>

#include "plato/mesh/MeshDesignVariables.hpp"
#include "plato/mesh/MeshDesignVariablesDensitiesViewIterator.hpp"

namespace plato::mesh
{
/// @brief The purpose of this object is to provide an interface for the density field in MeshDesignVariables.
///
/// Its main use is for facilitating copying density data from MeshDesignVariables to some other data structure,
/// such as that used by an external physics app.
/// The iterator range it provides will iterate over the contained entity global IDs in ascending order, and only
/// visit each once, even if that entity is shared between blocks. For example, nodes may be shared between blocks
/// on any shared boundaries and so the same density value associated with a node may be in two or more blocks.
/// @note This is not meant to be used directly, instead use MeshDesignVariablesDensitiesView or
/// MeshDesignVariablesDensitiesMutableView.
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

}  // namespace detail

}  // namespace plato::mesh

#endif
