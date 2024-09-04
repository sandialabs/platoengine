#ifndef PLATO_DESIGN_VARIABLES_MESHDESIGNVARIABLESRANDOMACCESSVIEW
#define PLATO_DESIGN_VARIABLES_MESHDESIGNVARIABLESRANDOMACCESSVIEW

#include <functional>
#include <optional>

#include "plato/design_variables/MeshDesignVariables.hpp"

namespace plato::design_variables
{
/// @brief Provides const random access to MeshDesignVariables using the global ID as the index.
///
/// This performs a search over the sorted design variable vectors, which is O(log N).
struct MeshDesignVariablesRandomAccessView
{
    std::reference_wrapper<const MeshDesignVariables> mMeshDesignVariables;

    [[nodiscard]] auto size() const -> std::size_t;
    [[nodiscard]] auto operator[](ScalarFieldValue::IndexType aIndex) const -> std::optional<ScalarFieldValue>;
};

}  // namespace plato::design_variables

#endif
