#include "plato/criteria/extension/QuadraticObjective.hpp"

#include <numeric>
#include <vector>

#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"
#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/third_party_integration/common/Vector3.hpp"
#include "plato/utilities/ContainerHelpers.hpp"

namespace plato::criteria::extension
{
namespace
{
using Registration =
    library::CriterionRegistration<library::Parallelization::kSerial, library::FunctionDimension::kScalar>;

const auto kConfiguration = services::CriterionConfiguration{
    .mName = std::string{QuadraticObjective::kCriterionName}, .mIsParallelized = false, .mIsScalar = true};

[[maybe_unused]] static auto kQuadraticObjectiveRegistration = Registration{
    library::builtin_criterion_registration_name(QuadraticObjective::kCriterionName), [](const library::CriterionInput&)
    {
        return library::FunctionWithConfiguration{.mFunction = make_quadratic_objective_function(),
                                                  .mConfiguration = kConfiguration};
    }};

[[nodiscard]] auto coordinate_analysis_domain_mesh(
    const analysis::AnalysisDomainMeshSequentialView& aAnalysisDomainMeshSequentialView) -> bool
{
    return aAnalysisDomainMeshSequentialView.size() == 0U;
}

}  // namespace

auto QuadraticObjective::value(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const -> double
{
    const auto tView = analysis::AnalysisDomainMeshSequentialView{aAnalysisDomainMesh};
    if (coordinate_analysis_domain_mesh(tView))
    {
        const auto tNodalCoordinates = third_party_integration::common::flatten(
            mesh::EntityRetrieval{mesh::Mesh{aAnalysisDomainMesh}}.nodalCoordinates());
        return 0.5 * std::accumulate(tNodalCoordinates.begin(), tNodalCoordinates.end(), 0.0,
                                     [](const double& aSum, const auto& aValue)
                                     { return aSum + std::pow(aValue, 2.0); });
    }
    return 0.5 *
           std::accumulate(
               tView.begin(), tView.end(), 0.0, [](const double& aSum, const auto& aScalarFieldValue)
               { return aSum + std::pow(static_cast<analysis::ScalarFieldValue>(aScalarFieldValue).mValue, 2.0); });
}

auto QuadraticObjective::gradient(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const
    -> linear_algebra::DynamicVector<double>
{
    const auto tView = analysis::AnalysisDomainMeshSequentialView{aAnalysisDomainMesh};
    if (coordinate_analysis_domain_mesh(tView))
    {
        return linear_algebra::DynamicVector<double>{third_party_integration::common::flatten(
            mesh::EntityRetrieval{mesh::Mesh{aAnalysisDomainMesh}}.nodalCoordinates())};
    }
    auto tGradient = utilities::reserved_container<std::vector<double>>(tView.size());
    std::transform(tView.begin(), tView.end(), std::back_inserter(tGradient), [](const auto& aScalarFieldValue)
                   { return static_cast<analysis::ScalarFieldValue>(aScalarFieldValue).mValue; });
    return linear_algebra::DynamicVector<double>{std::move(tGradient)};
}

auto make_quadratic_objective_function() -> library::CriterionFunction
{
    return core::make_function_with_first_derivative([](const analysis::AnalysisDomainMesh& aAnalysisDomainMeshmesh)
                                                     { return QuadraticObjective{}.value(aAnalysisDomainMeshmesh); },
                                                     [](const analysis::AnalysisDomainMesh& aAnalysisDomainMeshmesh) {
                                                         return QuadraticObjective{}.gradient(aAnalysisDomainMeshmesh);
                                                     });
}
}  // namespace plato::criteria::extension
