#include <gtest/gtest.h>

#include "plato/geometry/extension/BrickShapeGeometry.hpp"
#include "plato/integration_tests/test_mass_objective/MassObjective.hpp"
#include "plato/utilities/STKUtilities.hpp"

namespace plato::integration_tests::serial
{
TEST(MassObjective, Value)
{
    namespace pfu = plato::utilities;
    namespace pfittmo = plato::integration_tests::test_mass_objective;

    constexpr double tDensity = 2.0;
    constexpr double tTarget = 1.0;
    const auto tMassObjective = pfittmo::MassObjective{tDensity, tTarget};

    constexpr std::string_view tMeshName = "massTest.exo";
    pfu::write_mesh(tMeshName, pfu::create_mesh("generated:1x1x1|bbox:-1,-1,-1,1,1,1"));

    constexpr double tExpectedMass = tDensity * 8.0;
    EXPECT_DOUBLE_EQ(tMassObjective.mass(tMeshName), tExpectedMass);
}

TEST(MassObjective, NumMeshNodes)
{
    namespace pfu = plato::utilities;
    namespace pfittmo = plato::integration_tests::test_mass_objective;

    const auto tMassObjective = pfittmo::MassObjective{0.0, 0.0};

    constexpr std::string_view tMeshName = "massTest.exo";
    pfu::write_mesh(tMeshName, pfu::create_mesh("generated:1x2x1|bbox:-1,-1,-1,1,1,1"));

    constexpr unsigned int tExpectedNumNodes = 12;
    EXPECT_DOUBLE_EQ(tMassObjective.numMeshNodes(tMeshName), tExpectedNumNodes);
}
}  // namespace plato::integration_tests::serial