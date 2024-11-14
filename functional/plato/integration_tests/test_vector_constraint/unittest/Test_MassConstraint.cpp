#include <gtest/gtest.h>

#include "plato/integration_tests/test_vector_constraint/MassConstraint.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"

namespace plato::integration_tests::test_vector_constraint::unittest
{

using third_party_integration::stk_io::test_utilities::OneBlock3x1x1HexMesh;

TEST_F(OneBlock3x1x1HexMesh, MassConstraintMasses)
{
    constexpr double tDensity = 2.3;
    const auto tMassConstraint = MassConstraint{tDensity};
    const std::vector<double> tExpectedMasses(mExpectedNumberOfElements, tDensity);
    const auto tResult = tMassConstraint.masses(std::string{mMeshFilePath});
    ASSERT_EQ(tResult.size(), 3U);

    EXPECT_DOUBLE_EQ(tResult[0], tExpectedMasses[0]);
    EXPECT_DOUBLE_EQ(tResult[1], tExpectedMasses[1]);
    EXPECT_DOUBLE_EQ(tResult[2], tExpectedMasses[2]);
}

}  // namespace plato::integration_tests::test_vector_constraint::unittest
