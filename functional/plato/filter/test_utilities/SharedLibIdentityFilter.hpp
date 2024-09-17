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
class SharedLibIdentityFilter : public library::FilterInterface
{
   public:
    [[nodiscard]] analysis::AnalysisDomainMesh filter(
        const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const override;

    [[nodiscard]] linear_algebra::DynamicVector<double> jacobianTimesVector(
        const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
        const linear_algebra::DynamicVector<double>& aV) const override;
};
}  // namespace plato::filter::test_utilities

#endif
