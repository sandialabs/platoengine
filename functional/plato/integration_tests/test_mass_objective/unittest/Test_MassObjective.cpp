#include <gtest/gtest.h>

#include <filesystem>

#include "plato/integration_tests/test_mass_objective/MassObjective.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/Utilities.hpp"

namespace plato::integration_tests::test_mass_objective::unittest
{
TEST(MassObjective, Value)
{
    constexpr double tDensity = 2.0;
    const auto tMassObjective = MassObjective{tDensity};

    constexpr std::string_view tMeshName = "massTest.exo";
    const third_party_integration::stk_io::CommandGenerator tCommandGenerator{
        {1, 1, 1}, {-1, -1, -1}, {1, 1, 1}, utilities::CommandElementType::Hex};
    third_party_integration::stk_io::write_mesh(
        tMeshName, third_party_integration::stk_io::generate_mesh(tCommandGenerator.toString()));

    constexpr double tExpectedMass = tDensity * 8.0;
    EXPECT_DOUBLE_EQ(tMassObjective.mass(tMeshName), tExpectedMass);

    std::filesystem::remove(tMeshName);
}

}  // namespace plato::integration_tests::test_mass_objective::unittest
