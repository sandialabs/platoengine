#ifndef PLATO_MESH_MESHDESIGNVARIABLESRANDOMACCESSVIEW
#define PLATO_MESH_MESHDESIGNVARIABLESRANDOMACCESSVIEW

#include <functional>
#include <optional>

#include "plato/mesh/MeshDesignVariables.hpp"

namespace plato::mesh
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

}  // namespace plato::mesh

#endif
