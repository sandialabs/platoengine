#include "plato/integration_tests/test_mass_objective/MassObjectiveInterface.hpp"

#include "plato/integration_tests/test_mass_objective/MassObjective.hpp"
#include "plato/third_party_integration/stk_io/Utilities.hpp"

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
    const unsigned int tGradientSize =
        third_party_integration::stk_io::read_mesh_node_size(aMeshProxy.mFileName.string()) * tNumDimensions;
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
