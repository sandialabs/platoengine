#include "plato/integration_tests/test_mass_criteria/MassPropertiesVectorInterface.hpp"

#include <cassert>

namespace plato::integration_tests::test_mass_criteria
{
namespace
{
constexpr auto kCriterionDimension = 4U;
}

auto MassPropertiesInterface::value(const analysis::AnalysisDomainMesh&) const -> std::vector<double>
{
    return {0.0, 1.0, 2.0, 3.0};
}

auto MassPropertiesInterface::rowVectorTimesJacobian(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                                     const std::vector<double>& aDirectionVector) const
    -> std::vector<double>
{
    assert(aAnalysisDomainMesh.mBlockScalarField.size() == 1U);
    assert(aDirectionVector.size() == kCriterionDimension);

    const auto tOptimizationVectorDimension = aAnalysisDomainMesh.mBlockScalarField.begin()->second.size();
    return std::vector<double>(tOptimizationVectorDimension);
}

auto MassPropertiesInterface::rowVectorTimesAdjointJacobian(const analysis::AnalysisDomainMesh&,
                                                            const std::vector<double>&) const -> std::vector<double>
{
    return std::vector<double>(kCriterionDimension);
}

}  // namespace plato::integration_tests::test_mass_criteria
auto plato_create_mass_properties_vector_criterion(const std::vector<std::string>&)
    -> std::unique_ptr<::plato::criteria::library::VectorCriterionInterface>
{
    return std::make_unique<plato::integration_tests::test_mass_criteria::MassPropertiesInterface>();
}
