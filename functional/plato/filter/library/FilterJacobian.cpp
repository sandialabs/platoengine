#include "plato/filter/library/FilterJacobian.hpp"

#include <cassert>

#include "plato/filter/library/FilterInterface.hpp"

namespace plato::filter::library
{
auto make_filter_jacobian(std::shared_ptr<FilterInterface> aFilter, analysis::AnalysisDomainMesh aAnalysisDomainMesh)
    -> linear_algebra::JacobianMultiplier
{
    assert(aFilter);
    return linear_algebra::JacobianMultiplier{
        [tFilter = std::move(aFilter),
         tAnalysisDomainMesh = std::move(aAnalysisDomainMesh)](const linear_algebra::DynamicVector<double>& aRowVector)
        { return tFilter->rowVectorTimesJacobian(tAnalysisDomainMesh, aRowVector); }};
}

auto make_filter_adjoint_jacobian(std::shared_ptr<FilterInterface> aFilter,
                                  analysis::AnalysisDomainMesh aAnalysisDomainMesh)
    -> linear_algebra::AdjointJacobianMultiplier
{
    assert(aFilter);
    return linear_algebra::AdjointJacobianMultiplier{linear_algebra::JacobianMultiplier{
        [tFilter = std::move(aFilter),
         tAnalysisDomainMesh = std::move(aAnalysisDomainMesh)](const linear_algebra::DynamicVector<double>& aRowVector)
        { return tFilter->rowVectorTimesAdjointJacobian(tAnalysisDomainMesh, aRowVector); }}};
}

}  // namespace plato::filter::library
