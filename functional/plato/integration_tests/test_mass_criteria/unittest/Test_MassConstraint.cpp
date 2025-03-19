#include <gtest/gtest.h>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/integration_tests/test_mass_criteria/MassConstraint.hpp"
#include "plato/integration_tests/test_mass_criteria/MassConstraintVectorInterface.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"

namespace plato::integration_tests::test_mass_criteria::unittest
{
namespace
{
constexpr double tDensity = 2.3;
}

using third_party_integration::stk_io::test_utilities::OneBlock3x1x1HexMesh;

TEST_F(OneBlock3x1x1HexMesh, MassConstraintMasses)
{
    const auto tMassConstraint = MassConstraint{tDensity};
    const auto tExpectedMasses = std::vector(mExpectedNumberOfElements, tDensity);
    const auto tResult = tMassConstraint.masses(std::string{mMeshFilePath});
    ASSERT_EQ(tResult.size(), 3U);

    EXPECT_DOUBLE_EQ(tResult[0], tExpectedMasses[0]);
    EXPECT_DOUBLE_EQ(tResult[1], tExpectedMasses[1]);
    EXPECT_DOUBLE_EQ(tResult[2], tExpectedMasses[2]);
}

TEST_F(OneBlock3x1x1HexMesh, RowVectorTimesJacobian)
{
    const auto tAnalysisMesh = analysis::AnalysisDomainMesh{mMeshFilePath, {}};
    const auto tRowVector = std::vector<double>();
    const auto tResultProduct = MassConstraintInterface{}.rowVectorTimesJacobian(tAnalysisMesh, tRowVector);
    const auto tExpectedProduct = std::vector(mExpectedNumberOfElements, 0.0);
    EXPECT_EQ(tResultProduct, tExpectedProduct);
}

TEST_F(OneBlock3x1x1HexMesh, RowVectorTimesAdjointJacobian)
{
    const auto tFieldValues = analysis::AnalysisDomainMesh::ScalarFieldVector(mCommandGenerator.numberOfNodes());
    constexpr auto tBlockID = 1U;
    const auto tAnalysisMesh = analysis::AnalysisDomainMesh{mMeshFilePath, {{tBlockID, tFieldValues}}};
    const auto tRowVector = std::vector<double>();
    const auto tResultProduct = MassConstraintInterface{}.rowVectorTimesAdjointJacobian(tAnalysisMesh, tRowVector);
    const auto tExpectedProduct = std::vector(tFieldValues.size(), 0.0);
    EXPECT_EQ(tResultProduct, tExpectedProduct);
}

}  // namespace plato::integration_tests::test_mass_criteria::unittest
