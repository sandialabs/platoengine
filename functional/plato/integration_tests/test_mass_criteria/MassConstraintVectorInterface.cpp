#include "plato/integration_tests/test_mass_criteria/MassConstraintVectorInterface.hpp"

#include <algorithm>
#include <cassert>
#include <memory>
#include <string>
#include <vector>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/criteria/library/VectorCriterionInterface.hpp"
#include "plato/integration_tests/test_mass_criteria/MassConstraint.hpp"

namespace plato::integration_tests::test_mass_criteria
{

auto MassConstraintInterface::value(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const
    -> std::vector<double>
{
    const MassConstraint tMassConstraint{kDensity};
    return tMassConstraint.masses(std::string{aAnalysisDomainMesh.mFileName});
}

auto MassConstraintInterface::rowVectorTimesJacobian(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                                     const std::vector<double>&) const -> std::vector<double>
{
    auto tMasses = value(aAnalysisDomainMesh);
    std::fill(tMasses.begin(), tMasses.end(), 0.0);
    return tMasses;
}

auto MassConstraintInterface::rowVectorTimesAdjointJacobian(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                                            const std::vector<double>&) const -> std::vector<double>
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
