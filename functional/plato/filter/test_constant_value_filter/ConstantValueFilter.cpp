#include "plato/filter/test_constant_value_filter/ConstantValueFilter.hpp"

#include <cstddef>
#include <memory>
#include <vector>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"
#include "plato/filter/library/FilterInterface.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"

namespace plato::filter::test_constant_value_filter
{
auto ConstantValueFilter::filter(const analysis::AnalysisDomainMesh& /*aAnalysisDomainMesh*/) const
    -> analysis::AnalysisDomainMesh
{
    auto tValues = std::vector{-1.0, 0.0, 1.0};
    auto tIDs = std::vector<std::size_t>{1U, 2U, 3U};
    constexpr auto tBlockId = 1U;
    const auto tMeshName = std::filesystem::path{};
    return analysis::AnalysisDomainMesh{tMeshName,
                                        analysis::AnalysisDomainMesh::BlockScalarField{
                                            {tBlockId, analysis::combine_scalar_field_values_and_ids(tValues, tIDs)}}};
}

auto ConstantValueFilter::rowVectorTimesJacobian(const analysis::AnalysisDomainMesh& /*aAnalysisDomainMesh*/,
                                                 const linear_algebra::DynamicVector<double>& /*aRowVector*/) const
    -> linear_algebra::DynamicVector<double>
{
    const auto tArbitraryVector = linear_algebra::DynamicVector{10.0, 11.0, 12.0};
    return tArbitraryVector;
}

auto ConstantValueFilter::rowVectorTimesAdjointJacobian(
    const analysis::AnalysisDomainMesh& /*aAnalysisDomainMesh*/,
    const plato::linear_algebra::DynamicVector<double>& /*aRowVector*/) const
    -> plato::linear_algebra::DynamicVector<double>
{
    const auto tArbitraryVector = linear_algebra::DynamicVector{9.0, 8.0, 7.0};
    return tArbitraryVector;
}

}  // namespace plato::filter::test_constant_value_filter

namespace plato
{
auto plato_create_filter(const filter::library::FilterParameters& /*unused*/)
    -> std::unique_ptr<filter::library::FilterInterface>
{
    return std::make_unique<filter::test_constant_value_filter::ConstantValueFilter>();
}
}  // namespace plato
