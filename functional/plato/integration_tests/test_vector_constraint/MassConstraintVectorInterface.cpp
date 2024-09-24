#include "plato/integration_tests/test_vector_constraint/MassConstraintVectorInterface.hpp"

#include <iostream>
#include <numeric>
#include <string_view>

#include "plato/integration_tests/test_vector_constraint/MassConstraint.hpp"

namespace plato::integration_tests::test_vector_constraint
{

std::vector<double> MassConstraintInterface::value(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const
{
    std::cout << "Value inside shared object" << std::endl;

    const MassConstraint tMassConstraint{kDensity};
    return tMassConstraint.masses(std::string{aAnalysisDomainMesh.mFileName});
}

std::vector<double> MassConstraintInterface::jacobianTimesVector(
    const analysis::AnalysisDomainMesh& /*aAnalysisDomainMesh*/, const std::vector<double>& aDirectionVector) const
{
    std::cout << "J*V inside shared object" << std::endl;
    return aDirectionVector;
}

std::vector<double> MassConstraintInterface::adjointJacobianTimesVector(
    const analysis::AnalysisDomainMesh& /*aAnalysisDomainMesh*/, const std::vector<double>& aDualVector) const
{
    std::cout << "J^T*D  inside shared object" << std::endl;
    return {std::accumulate(aDualVector.begin(), aDualVector.end(), 0.0)};
}

}  // namespace plato::integration_tests::test_vector_constraint

std::unique_ptr<::plato::criteria::library::VectorCriterionInterface> plato_create_criterion(
    const std::vector<std::string>&)
{
    return std::make_unique<::plato::integration_tests::test_vector_constraint::MassConstraintInterface>();
}
