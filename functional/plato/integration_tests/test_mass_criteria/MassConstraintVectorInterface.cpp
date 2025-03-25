#include "plato/integration_tests/test_mass_criteria/MassConstraintVectorInterface.hpp"

#include <cassert>
#include <numeric>

#include "plato/integration_tests/test_mass_criteria/MassConstraint.hpp"

namespace plato::integration_tests::test_mass_criteria
{

std::vector<double> MassConstraintInterface::value(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const
{
    const MassConstraint tMassConstraint{kDensity};
    return tMassConstraint.masses(std::string{aAnalysisDomainMesh.mFileName});
}

std::vector<double> MassConstraintInterface::rowVectorTimesJacobian(
    const analysis::AnalysisDomainMesh& aAnalysisDomainMesh, const std::vector<double>&) const
{
    auto tMasses = value(aAnalysisDomainMesh);
    std::fill(tMasses.begin(), tMasses.end(), 0.0);
    return tMasses;
}

std::vector<double> MassConstraintInterface::rowVectorTimesAdjointJacobian(
    const analysis::AnalysisDomainMesh& aAnalysisDomainMesh, const std::vector<double>&) const
{
    assert(aAnalysisDomainMesh.mBlockScalarField.size() == 1U);
    const auto tOptimizationVectorDimension = aAnalysisDomainMesh.mBlockScalarField.begin()->second.size();
    return std::vector<double>(tOptimizationVectorDimension, 0.0);
}

}  // namespace plato::integration_tests::test_mass_criteria

auto plato_create_vector_criterion(const std::vector<std::string>&)
    -> std::unique_ptr<::plato::criteria::library::VectorCriterionInterface>
{
    return std::make_unique<::plato::integration_tests::test_mass_criteria::MassConstraintInterface>();
}

auto plato_create_vector_test_mass_criterion(const std::vector<std::string>&)
    -> std::unique_ptr<::plato::criteria::library::VectorCriterionInterface>
{
    return std::make_unique<::plato::integration_tests::test_mass_criteria::MassConstraintInterface>();
}
