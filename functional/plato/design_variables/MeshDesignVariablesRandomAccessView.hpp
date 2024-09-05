#ifndef PLATO_DESIGN_VARIABLES_MESHDESIGNVARIABLESRANDOMACCESSVIEW
#define PLATO_DESIGN_VARIABLES_MESHDESIGNVARIABLESRANDOMACCESSVIEW

#include <functional>
#include <optional>

#include "plato/design_variables/MeshDesignVariables.hpp"

namespace plato::design_variables
{
/// @brief Provides const random access via an `operator[]` to MeshDesignVariables using the global ID as the index.
struct MeshDesignVariablesRandomAccessView
{
    std::reference_wrapper<const MeshDesignVariables> mMeshDesignVariables;

    /// @brief The number of design variables contained in the MeshDesignVariables.
    [[nodiscard]] auto size() const -> std::size_t;

    /// @brief Const access to a ScalarFieldValue by global ID.
    /// @note The performs a search over the sorted design variable vectors, which is O(log N).
    /// @return `std::nullopt` if there is no entry associated with @a aGlobalID
    [[nodiscard]] auto operator[](ScalarFieldValue::IndexType aGlobalID) const -> std::optional<ScalarFieldValue>;
};

}  // namespace plato::design_variables

#endif
