#include <gtest/gtest.h>

#include <filesystem>

#include "plato/integration_tests/test_mass_objective/MassObjective.hpp"
#include "plato/utilities/STKCommandGenerator.hpp"
#include "plato/utilities/STKUtilities.hpp"

namespace plato::integration_tests::test_mass_objective::unittest
{
TEST(MassObjective, Value)
{
    constexpr double tDensity = 2.0;
    const auto tMassObjective = MassObjective{tDensity};

    constexpr std::string_view tMeshName = "massTest.exo";
    const utilities::STKCommandGenerator tSTKCommandGenerator{
        {1, 1, 1}, {-1, -1, -1}, {1, 1, 1}, utilities::STKCommandElementType::Hex};
    utilities::write_mesh(tMeshName, utilities::create_mesh(tSTKCommandGenerator.toString()));

    constexpr double tExpectedMass = tDensity * 8.0;
    EXPECT_DOUBLE_EQ(tMassObjective.mass(tMeshName), tExpectedMass);

    std::filesystem::remove(tMeshName);
}

}  // namespace plato::integration_tests::test_mass_objective::unittest
