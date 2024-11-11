#ifndef PLATO_FILTER_TESTUTILITIES_SHAREDLIBIDENTITYFILTER
#define PLATO_FILTER_TESTUTILITIES_SHAREDLIBIDENTITYFILTER

#include "plato/filter/library/FilterInterface.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"

namespace plato::analysis
{
struct AnalysisDomainMesh;
}

namespace plato::filter::test_utilities
{
/// @brief The purpose of this filter is to test the shared library interface.
///
/// It is an identity filter, meaning that the density field will be unchanged on
/// calls to `filter` and its Jacobian is the identity matrix.
// CPD-OFF
class SharedLibIdentityFilter : public library::FilterInterface
{
   public:
    [[nodiscard]] auto filter(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const
        -> analysis::AnalysisDomainMesh override;

    [[nodiscard]] auto rowVectorTimesJacobian(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                              const linear_algebra::DynamicVector<double>& aV) const
        -> linear_algebra::DynamicVector<double> override;

    [[nodiscard]] auto rowVectorTimesAdjointJacobian(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                                     const plato::linear_algebra::DynamicVector<double>& aV) const
        -> plato::linear_algebra::DynamicVector<double> override;
};
// CPD-ON
}  // namespace plato::filter::test_utilities

#endif
