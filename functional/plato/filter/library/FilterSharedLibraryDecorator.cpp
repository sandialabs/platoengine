#include "plato/filter/library/FilterSharedLibraryDecorator.hpp"

#include <iostream>

#include "plato/analysis/AnalysisDomainMesh.hpp"

namespace plato::filter::library
{
FilterSharedLibraryDecorator::FilterSharedLibraryDecorator(
    services::SharedLibraryObject<std::unique_ptr<FilterInterface>>&& aSharedLibraryObject)
    : mSharedLibraryObject{std::move(aSharedLibraryObject)}
{
}

auto FilterSharedLibraryDecorator::filter(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const
    -> analysis::AnalysisDomainMesh
{
    return mSharedLibraryObject.object()->filter(aAnalysisDomainMesh);
}

auto FilterSharedLibraryDecorator::rowVectorTimesJacobian(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                                          const linear_algebra::DynamicVector<double>& aRowVector) const
    -> linear_algebra::DynamicVector<double>
{
    return mSharedLibraryObject.object()->rowVectorTimesJacobian(aAnalysisDomainMesh, aRowVector);
}

auto FilterSharedLibraryDecorator::rowVectorTimesAdjointJacobian(
    const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
    const plato::linear_algebra::DynamicVector<double>& aRowVector) const
    -> plato::linear_algebra::DynamicVector<double>
{
    return mSharedLibraryObject.object()->rowVectorTimesAdjointJacobian(aAnalysisDomainMesh, aRowVector);
}
}  // namespace plato::filter::library
