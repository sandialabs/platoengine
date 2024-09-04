#ifndef PLATO_DESIGN_VARIABLES_MESHDESIGNVARIABLESSEQUENTIALVIEW
#define PLATO_DESIGN_VARIABLES_MESHDESIGNVARIABLESSEQUENTIALVIEW

#include <functional>
#include <optional>

#include "plato/design_variables/MeshDesignVariables.hpp"
#include "plato/design_variables/MeshDesignVariablesSequentialViewIterator.hpp"

namespace plato::design_variables
{
/// @brief The purpose of this object is to provide an interface for MeshDesignVariables scalar field in with std
/// algorithms.
///
/// Its main use is for facilitating copying field data from MeshDesignVariables to some other data structure,
/// such as that used by an external physics app.
/// The iterator range it provides will iterate over the contained entity global IDs in ascending order, and only
/// visit each once, even if that entity is shared between blocks. For example, nodes may be shared between blocks
/// on any shared boundaries and so the same field value associated with a node may be in two or more blocks.
/// @note This is not meant to be used directly, instead use MeshDesignVariablesSequentialView or
/// MeshDesignVariablesMutableSequentialView.
template <typename MeshDesignVariablesType>
struct MeshDesignVariablesSequentialViewTemplate
{
    std::reference_wrapper<MeshDesignVariablesType> mMeshDesignVariables;

    [[nodiscard]] std::size_t size() const;

    using IteratorType = typename detail::IteratorType<MeshDesignVariablesType>::type;
    [[nodiscard]] IteratorType begin() const;
    [[nodiscard]] IteratorType end() const;
};

using MeshDesignVariablesSequentialView = MeshDesignVariablesSequentialViewTemplate<const MeshDesignVariables>;
using MeshDesignVariablesMutableSequentialView = MeshDesignVariablesSequentialViewTemplate<MeshDesignVariables>;

/// @brief Converts the field values associated with the mesh in @a aMeshView to a `std::vector`.
[[nodiscard]] auto mesh_design_variables_to_vector(MeshDesignVariablesSequentialView aMeshView)
    -> std::vector<ScalarFieldValue>;

/// @brief Combines a vector of scalar field design values with a vector of global IDs into a single vector containing
/// ScalarFieldValue objects.
/// @warning This does not set the field `mDesignVariableVectorIndex` in the returned values. This is meant for use as
/// an initial step to combine global IDs and density values.
[[nodiscard]] auto combine_scalar_field_values_and_ids(const std::vector<double>& aScalarField,
                                                       const std::vector<std::size_t>& aIDs)
    -> std::vector<ScalarFieldValue>;

/// @brief Splits a vector of ScalarFieldValue objects into the density values and node/element map.
[[nodiscard]] auto split_scalar_field_values(const std::vector<ScalarFieldValue>& aScalarField)
    -> std::pair<std::vector<double>, std::vector<std::size_t>>;

}  // namespace plato::design_variables

#endif
