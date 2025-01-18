#ifndef PLATO_FILTER_TESTCONSTANTVALUEFILTER_CONSTANTVALUEFILTER
#define PLATO_FILTER_TESTCONSTANTVALUEFILTER_CONSTANTVALUEFILTER

#include "plato/filter/library/FilterInterface.hpp"

namespace plato::linear_algebra
{
template <typename T>
class DynamicVector;
}

namespace plato::analysis
{
struct AnalysisDomainMesh;
}

namespace plato::filter::test_constant_value_filter
{
/// @brief The purpose of this filter is to test the shared library interface.
///
/// It only returns constant, known values from its member functions so that it can be used to check
/// the interface in any wrapper classes, such as FilterSharedLibraryDecorator.
// CPD-OFF
class ConstantValueFilter : public library::FilterInterface
{
   public:
    /// @brief Always returns an AnalysisDomainMesh with one block and scalar field: [{1, -1}, {2, 0}, {3, 1}]
    [[nodiscard]] auto filter(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const
        -> analysis::AnalysisDomainMesh override;

    /// @brief Always returns a vector containing [10, 11, 12]
    [[nodiscard]] auto rowVectorTimesJacobian(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                              const linear_algebra::DynamicVector<double>& aV) const
        -> linear_algebra::DynamicVector<double> override;

    /// @brief Always returns a vector containing [9, 8, 7]
    [[nodiscard]] auto rowVectorTimesAdjointJacobian(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                                     const plato::linear_algebra::DynamicVector<double>& aV) const
        -> plato::linear_algebra::DynamicVector<double> override;
};
// CPD-ON
}  // namespace plato::filter::test_constant_value_filter

#endif
