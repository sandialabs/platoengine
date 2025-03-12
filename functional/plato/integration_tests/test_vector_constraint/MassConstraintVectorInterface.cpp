#include "plato/integration_tests/test_vector_constraint/MassConstraintVectorInterface.hpp"

#include <numeric>

#include "plato/integration_tests/test_vector_constraint/MassConstraint.hpp"

namespace plato::integration_tests::test_vector_constraint
{

std::vector<double> MassConstraintInterface::value(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const
{
    const MassConstraint tMassConstraint{kDensity};
    return tMassConstraint.masses(std::string{aAnalysisDomainMesh.mFileName});
}

std::vector<double> MassConstraintInterface::rowVectorTimesJacobian(
    const analysis::AnalysisDomainMesh& /*aAnalysisDomainMesh*/, const std::vector<double>& aDirectionVector) const
{
    return aDirectionVector;
}

std::vector<double> MassConstraintInterface::rowVectorTimesAdjointJacobian(
    const analysis::AnalysisDomainMesh& /*aAnalysisDomainMesh*/, const std::vector<double>& aDualVector) const
{
    return {std::accumulate(aDualVector.begin(), aDualVector.end(), 0.0)};
}

}  // namespace plato::integration_tests::test_vector_constraint

auto plato_create_criterion(const std::vector<std::string>&)
    -> std::unique_ptr<::plato::criteria::library::VectorCriterionInterface>
{
    return std::make_unique<::plato::integration_tests::test_vector_constraint::MassConstraintInterface>();
}
