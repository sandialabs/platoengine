#include "plato/filter/test_identity_filter/SharedLibIdentityFilter.hpp"

#include <memory>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/filter/library/FilterInterface.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"

namespace plato::filter::test_identity_filter
{
auto SharedLibIdentityFilter::filter(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const
    -> analysis::AnalysisDomainMesh
{
    return aAnalysisDomainMesh;
}

auto SharedLibIdentityFilter::rowVectorTimesJacobian(const analysis::AnalysisDomainMesh&,
                                                     const linear_algebra::DynamicVector<double>& aV) const
    -> linear_algebra::DynamicVector<double>
{
    return aV;
}

auto SharedLibIdentityFilter::rowVectorTimesAdjointJacobian(const analysis::AnalysisDomainMesh& aMesh,
                                                            const linear_algebra::DynamicVector<double>& aV) const
    -> linear_algebra::DynamicVector<double>
{
    return rowVectorTimesJacobian(aMesh, aV);
}

}  // namespace plato::filter::test_identity_filter

namespace plato
{
auto plato_create_filter(const filter::library::FilterParameters&) -> std::unique_ptr<filter::library::FilterInterface>
{
    return std::make_unique<filter::test_identity_filter::SharedLibIdentityFilter>();
}
}  // namespace plato
