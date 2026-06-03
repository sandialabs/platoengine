#include "plato/integration_tests/test_mass_criteria/MassObjectiveInterface.hpp"

#include <memory>
#include <string>
#include <vector>

#include "plato/criteria/library/CriterionInterface.hpp"
#include "plato/integration_tests/test_mass_criteria/MassObjective.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/Mesh.hpp"

namespace plato::integration_tests::test_mass_criteria
{
auto MassObjectiveInterface::value(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const -> double
{
    constexpr double tDensity = 1.0;
    const auto tMassObjective = MassObjective{tDensity};
    return tMassObjective.mass(aAnalysisDomainMesh.mFileName.string());
}

auto MassObjectiveInterface::gradient(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const
    -> std::vector<double>
{
    ///@todo Populate the gradient with actual values
    constexpr unsigned int tNumDimensions = 3;
    const unsigned int tGradientSize =
        mesh::EntityCounts{mesh::Mesh{aAnalysisDomainMesh.mFileName}}.numberOfNodes() * tNumDimensions;

    return std::vector<double>(tGradientSize, 1.0);
}
}  // namespace plato::integration_tests::test_mass_criteria

auto plato_create_criterion(const std::vector<std::string>& /*unused*/)
    -> std::unique_ptr<::plato::criteria::library::CriterionInterface>
{
    return std::make_unique<::plato::integration_tests::test_mass_criteria::MassObjectiveInterface>();
}

auto plato_create_test_mass_criterion(const std::vector<std::string>& /*unused*/)
    -> std::unique_ptr<::plato::criteria::library::CriterionInterface>
{
    return std::make_unique<::plato::integration_tests::test_mass_criteria::MassObjectiveInterface>();
}
