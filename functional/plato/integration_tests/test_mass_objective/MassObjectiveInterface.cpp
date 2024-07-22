#include "plato/integration_tests/test_mass_objective/MassObjectiveInterface.hpp"

#include "plato/integration_tests/test_mass_objective/MassObjective.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/Mesh.hpp"

namespace plato::integration_tests::test_mass_objective
{
double MassObjectiveInterface::value(const mesh::MeshProxy& aMeshProxy) const
{
    constexpr double tDensity = 1.0;
    const auto tMassObjective = MassObjective{tDensity};
    return tMassObjective.mass(aMeshProxy.mFileName.string());
}

std::vector<double> MassObjectiveInterface::gradient(const mesh::MeshProxy& aMeshProxy) const
{
    ///@todo Populate the gradient with actual values
    constexpr unsigned int tNumDimensions = 3;
    const unsigned int tGradientSize =
        mesh::EntityCounts{mesh::Mesh{aMeshProxy.mFileName}}.numberOfNodes() * tNumDimensions;

    return std::vector<double>(tGradientSize, 1.0);
}
}  // namespace plato::integration_tests::test_mass_objective

std::unique_ptr<::plato::criteria::library::CriterionInterface> plato_create_criterion(const std::vector<std::string>&)
{
    return std::make_unique<::plato::integration_tests::test_mass_objective::MassObjectiveInterface>();
}

std::unique_ptr<::plato::criteria::library::CriterionInterface> plato_create_test_mass_criterion(
    const std::vector<std::string>&)
{
    return std::make_unique<::plato::integration_tests::test_mass_objective::MassObjectiveInterface>();
}
