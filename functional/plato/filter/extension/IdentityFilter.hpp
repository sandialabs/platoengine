#ifndef PLATO_FILTER_EXTENSION_IDENTITYFILTER
#define PLATO_FILTER_EXTENSION_IDENTITYFILTER

#include "plato/filter/library/FilterInterface.hpp"
#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"

// clang-format off
PLATO_FILTER_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), identity_filter,
    (double, filter_radius, "Do not use - Identity filter cannot have a filter radius.")
)
// clang-format on

namespace plato::analysis
{
struct AnalysisDomainMesh;
}

namespace plato::filter::extension
{
/// @brief A Filter that does not alter the density field, mostly used for testing.
///
/// Calls to the filter member function return the same density field,
/// and the Jacobian is the identity matrix.
// CPD-OFF
class IdentityFilter : public library::FilterInterface
{
   public:
    [[nodiscard]] auto filter(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const
        -> analysis::AnalysisDomainMesh override;

    [[nodiscard]] auto rowVectorTimesJacobian(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                              const linear_algebra::DynamicVector<double>& aV) const
        -> linear_algebra::DynamicVector<double> override;

    [[nodiscard]] auto rowVectorTimesAdjointJacobian(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                                     const linear_algebra::DynamicVector<double>& aV) const
        -> linear_algebra::DynamicVector<double> override;
};
// CPD-ON

/// @brief Generates a input struct for IdentityFilter, useful for tests.
[[nodiscard]] auto create_valid_identity_filter_input() -> input_parser::identity_filter;

[[nodiscard]] auto make_identity_filter_function() -> library::FilterFunction;

[[nodiscard]] auto validate_identity_filter(const input_parser::identity_filter& aInput) -> std::optional<std::string>;

}  // namespace plato::filter::extension

#endif
