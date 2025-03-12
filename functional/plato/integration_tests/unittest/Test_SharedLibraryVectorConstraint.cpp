#include <gtest/gtest.h>

#include <filesystem>
#include <iterator>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/criteria/extension/SharedLibraryVectorCriterion.hpp"
#include "plato/integration_tests/test_vector_constraint/MassConstraintVectorInterface.hpp"
#include "plato/integration_tests/utilities/AppConfigurationTestUtilities.hpp"
#include "plato/test_utilities/Containers.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"

namespace plato::integration_tests::serial
{
using third_party_integration::stk_io::test_utilities::OneBlock3x1x1HexMesh;
namespace
{
// These tests use the mass objective library but through the shared library interface so they are testing the
// shared library more generically
constexpr std::string_view kLibPath = "libPlatoTestVectorConstraint.so";

[[nodiscard]] auto test_shared_library_criterion() -> criteria::extension::SharedLibraryVectorCriterion
{
    const auto tTestConfiguration = utilities::test_vector_app_configuration(kLibPath);
    return criteria::extension::SharedLibraryVectorCriterion{
        tTestConfiguration, tTestConfiguration.mConfiguration.mCriteria.front(), {}};
}

[[nodiscard]] auto create_n_step_vector(const unsigned int aSize) -> std::vector<double>
{
    std::vector<double> tVector;
    tVector.reserve(aSize);
    const double tStep = 1.0 / aSize;
    const double tStart = 0;
    std::generate_n(std::back_inserter(tVector), aSize,
                    [tStart, tStep, aN = 0]() mutable
                    {
                        aN++;
                        return tStart + tStep * aN;
                    });
    return tVector;
}

}  // namespace

TEST_F(OneBlock3x1x1HexMesh, SharedLibraryCallValue)
{
    const auto tSharedLibrary = test_shared_library_criterion();
    const auto tMasses = tSharedLibrary.value(analysis::AnalysisDomainMesh{mMeshFilePath, {}}).stdVector();

    const auto tExpected = std::vector<double>(mExpectedNumberOfElements, test_vector_constraint::kDensity);
    constexpr auto tTolerance = 1e-15;
    test_utilities::expect_container_entries_near(tExpected, tMasses, tTolerance,
                                                  TEST_CONTEXT("Vector mass density value"));
}

TEST_F(OneBlock3x1x1HexMesh, MakeCriterionFunctionAndCallValue) {}

TEST_F(OneBlock3x1x1HexMesh, MakeConstraintAndCallValue) {}

TEST_F(OneBlock3x1x1HexMesh, SharedLibraryCallJacobianTimesVector)
{
    const auto tSharedLibrary = test_shared_library_criterion();
    const linear_algebra::DynamicVector<double> tDirection(create_n_step_vector(mExpectedNumberOfElements));
    const auto tJacobianTimesVector =
        tSharedLibrary.rowVectorTimesJacobian(analysis::AnalysisDomainMesh{mMeshFilePath, {}}, tDirection).stdVector();

    const auto tGold = tDirection.stdVector();
    EXPECT_EQ(tJacobianTimesVector, tGold);
}

TEST_F(OneBlock3x1x1HexMesh, SharedLibraryCallAdjointJacobianTimesVector)
{
    const auto tSharedLibrary = test_shared_library_criterion();
    const linear_algebra::DynamicVector<double> tDual(create_n_step_vector(mExpectedNumberOfElements));
    const auto tAdjointJacobianTimesDual =
        tSharedLibrary.rowVectorTimesAdjointJacobian(analysis::AnalysisDomainMesh{mMeshFilePath, {}}, tDual)
            .stdVector();

    const auto tSum = std::accumulate(tDual.stdVector().begin(), tDual.stdVector().end(), 0.0);
    const std::vector<double> tGold(1, tSum);
    EXPECT_EQ(tAdjointJacobianTimesDual, tGold);
}

}  // namespace plato::integration_tests::serial
