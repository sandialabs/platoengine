#ifndef PLATO_ANALYSIS_MESHDESIGNVARIABLESRANDOMACCESSVIEW
#define PLATO_ANALYSIS_MESHDESIGNVARIABLESRANDOMACCESSVIEW

#include <functional>
#include <optional>

#include "plato/analysis/AnalysisDomainMesh.hpp"

namespace plato::analysis
{
/// @brief Provides const random access via an `operator[]` to AnalysisDomainMesh using the global ID as the index.
struct AnalysisDomainMeshRandomAccessView
{
    std::reference_wrapper<const AnalysisDomainMesh> mAnalysisDomainMesh;

    /// @brief The number of design variables contained in the AnalysisDomainMesh.
    [[nodiscard]] auto size() const -> std::size_t;

    /// @brief Const access to a ScalarFieldValue by global ID.
    /// @note The performs a search over the sorted design variable vectors, which is O(log N).
    /// @return `std::nullopt` if there is no entry associated with @a aGlobalID
    [[nodiscard]] auto operator[](ScalarFieldValue::IndexType aGlobalID) const -> std::optional<ScalarFieldValue>;
};

}  // namespace plato::analysis

#endif
