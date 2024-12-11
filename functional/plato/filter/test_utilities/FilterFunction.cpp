#include "plato/filter/test_utilities/FilterFunction.hpp"

#include "plato/analysis/AnalysisDomainMesh.hpp"

namespace plato::filter::test_utilities
{
auto make_filter_function(const std::shared_ptr<library::FilterInterface>& aFilter) -> library::FilterFunction
{
    return library::FilterFunction{[aFilter](const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
                                   { return aFilter->filter(aAnalysisDomainMesh); },
                                   [aFilter](const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
                                   { return library::make_filter_jacobian(aFilter, aAnalysisDomainMesh); },
                                   [aFilter](const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
                                   { return library::make_filter_adjoint_jacobian(aFilter, aAnalysisDomainMesh); }};
}

}  // namespace plato::filter::test_utilities
