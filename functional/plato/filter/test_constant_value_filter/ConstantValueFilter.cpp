#include "plato/filter/test_constant_value_filter/ConstantValueFilter.hpp"

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"

namespace plato::filter::test_constant_value_filter
{
auto ConstantValueFilter::filter(const analysis::AnalysisDomainMesh& /*aAnalysisDomainMesh*/) const
    -> analysis::AnalysisDomainMesh
{
    auto tValues = std::vector{-1.0, 0.0, 1.0};
    auto tIDs = std::vector<std::size_t>{1U, 2U, 3U};
    constexpr auto kBlockID = 1U;
    constexpr auto kMeshName = std::string_view{""};
    return analysis::AnalysisDomainMesh{kMeshName,
                                        analysis::AnalysisDomainMesh::BlockScalarField{
                                            {kBlockID, analysis::combine_scalar_field_values_and_ids(tValues, tIDs)}}};
}

auto ConstantValueFilter::rowVectorTimesJacobian(const analysis::AnalysisDomainMesh& /*aAnalysisDomainMesh*/,
                                                 const linear_algebra::DynamicVector<double>& /*aV*/) const
    -> linear_algebra::DynamicVector<double>
{
    return linear_algebra::DynamicVector{10.0, 11.0, 12.0};
}

auto ConstantValueFilter::rowVectorTimesAdjointJacobian(
    const analysis::AnalysisDomainMesh& /*aAnalysisDomainMesh*/,
    const plato::linear_algebra::DynamicVector<double>& /*aV*/) const -> plato::linear_algebra::DynamicVector<double>
{
    return linear_algebra::DynamicVector{9.0, 8.0, 7.0};
}

}  // namespace plato::filter::test_constant_value_filter

namespace plato
{
std::unique_ptr<filter::library::FilterInterface> plato_create_filter(const filter::library::FilterParameters&)
{
    return std::make_unique<filter::test_constant_value_filter::ConstantValueFilter>();
}
}  // namespace plato
