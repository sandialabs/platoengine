#ifndef PLATO_FILTER_LIBRARY_FILTERINTERFACE
#define PLATO_FILTER_LIBRARY_FILTERINTERFACE

#include <memory>
#include <optional>
#include <string_view>

#include "plato/linear_algebra/DynamicVector.hpp"

namespace plato::analysis
{
struct AnalysisDomainMesh;
}

namespace plato::filter::library
{
static constexpr std::string_view kCreateFilterFunctionName = "plato_create_filter";

/// @brief Holds parameters defining a filter for construction.
struct FilterParameters
{
    double mFilterRadius = 0.0;
    std::optional<double> mBoundaryStickingPenalty = std::nullopt;
};

/// @brief Base class for density field topology filters.
///
/// Derived types must implement filter, which applies the specific
/// filter to the density field in a AnalysisDomainMesh, and rowVectorTimesJacobian,
/// which implements the left multiplication of the Jacobian of the filter
/// with a row vector, i.e. \f$v^T J\f$.
class FilterInterface
{
   public:
    FilterInterface() = default;
    virtual ~FilterInterface() = default;

    /// @brief Applies the filter to the density field held in @a aAnalysisDomainMesh
    [[nodiscard]] virtual auto filter(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const
        -> analysis::AnalysisDomainMesh = 0;

    /// @brief Implements multiplication of row vector @a aRowVector and the Jacobian of the
    ///  filter computed at the argument @a aAnalysisDomainMesh.
    [[nodiscard]] virtual auto rowVectorTimesJacobian(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                                      const linear_algebra::DynamicVector<double>& aRowVector) const
        -> linear_algebra::DynamicVector<double> = 0;

    ///@brief Evaluate the product of a row vector with the transpose of the Jacobian matrix.
    [[nodiscard]] virtual auto rowVectorTimesAdjointJacobian(
        const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
        const plato::linear_algebra::DynamicVector<double>& aRowVector) const
        -> plato::linear_algebra::DynamicVector<double> = 0;

    FilterInterface(const FilterInterface&) = delete;
    auto operator=(const FilterInterface&) -> FilterInterface& = delete;
    FilterInterface(FilterInterface&&) = delete;
    auto operator=(FilterInterface&&) -> FilterInterface& = delete;
};

}  // namespace plato::filter::library

namespace plato
{
extern "C" auto plato_create_filter(const filter::library::FilterParameters& aFilterParameters)
    -> std::unique_ptr<filter::library::FilterInterface>;
}
#endif
