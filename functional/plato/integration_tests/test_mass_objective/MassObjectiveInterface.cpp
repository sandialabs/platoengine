#include "plato/integration_tests/test_mass_objective/MassObjectiveInterface.hpp"

#include "plato/integration_tests/test_mass_objective/MassObjective.hpp"
#include "plato/utilities/STKUtilities.hpp"

namespace plato::integration_tests::test_mass_objective
{
double MassObjectiveInterface::value(const core::MeshProxy& aMeshProxy) const
{
    constexpr double tDensity = 1.0;
    const auto tMassObjective = MassObjective{tDensity};
    return tMassObjective.mass(aMeshProxy.mFileName.string());
}

std::vector<double> MassObjectiveInterface::gradient(const core::MeshProxy& aMeshProxy) const
{
    ///@todo Populate the gradient with actual values
    constexpr unsigned int tNumDimensions = 3;
    const unsigned int tGradientSize = utilities::read_mesh_node_size(aMeshProxy.mFileName.string()) * tNumDimensions;
    return std::vector<double>(tGradientSize, 1.0);
}
}  // namespace plato::integration_tests::test_mass_objective

namespace plato
{
std::unique_ptr<criteria::library::CriterionInterface> plato_create_criterion(const std::vector<std::string>&)
{
    return std::make_unique<integration_tests::test_mass_objective::MassObjectiveInterface>();
}
}  // namespace plato
