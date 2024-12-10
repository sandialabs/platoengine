#include "plato/filter/test_identity_filter/SharedLibIdentityFilter.hpp"

#include "plato/analysis/AnalysisDomainMesh.hpp"

namespace plato::filter::test_identity_filter
{
analysis::AnalysisDomainMesh SharedLibIdentityFilter::filter(
    const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const
{
    return aAnalysisDomainMesh;
}

linear_algebra::DynamicVector<double> SharedLibIdentityFilter::rowVectorTimesJacobian(
    const analysis::AnalysisDomainMesh&, const linear_algebra::DynamicVector<double>& aV) const
{
    return aV;
}

linear_algebra::DynamicVector<double> SharedLibIdentityFilter::rowVectorTimesAdjointJacobian(
    const analysis::AnalysisDomainMesh& aMesh, const linear_algebra::DynamicVector<double>& aV) const
{
    return rowVectorTimesJacobian(aMesh, aV);
}

}  // namespace plato::filter::test_identity_filter

namespace plato
{
std::unique_ptr<filter::library::FilterInterface> plato_create_filter(const filter::library::FilterParameters&)
{
    return std::make_unique<filter::test_identity_filter::SharedLibIdentityFilter>();
}
}  // namespace plato
