#include <gtest/gtest.h>

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <iterator>
#include <utility>
#include <vector>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/components/ComponentType.hpp"
#include "plato/core/Function.hpp"
#include "plato/criteria/library/ConstraintFactory.hpp"
#include "plato/criteria/library/CriterionFactory.hpp"
#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/criteria/library/SharedLibraryVectorCriterion.hpp"
#include "plato/integration_tests/test_mass_criteria/MassConstraintVectorInterface.hpp"
#include "plato/integration_tests/utilities/AppConfigurationTestUtilities.hpp"
#include "plato/integration_tests/utilities/MassAppTestUtilities.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/test_utilities/Containers.hpp"
#include "plato/test_utilities/CoutCerrPrintTestFixture.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"

namespace plato::integration_tests::serial
{
namespace
{

using SharedLibraryVectorConstraintMeshFixture = third_party_integration::stk_io::test_utilities::OneBlock3x1x1HexMesh;
using SharedLibraryVectorConstraintCoutFixture = test_utilities::CoutCerrPrintTestFixture;

[[nodiscard]] auto test_shared_library_criterion() -> criteria::library::SharedLibraryVectorCriterion
{
    const auto tTestConfiguration = utilities::test_app_configuration(utilities::mass_app_library_file_name());
    const auto tVectorCriterion = std::find_if(tTestConfiguration.mConfiguration.mCriteria.cbegin(),
                                               tTestConfiguration.mConfiguration.mCriteria.cend(),
                                               [](const auto& aCriterion) { return !aCriterion.mIsScalar; });
    assert(tVectorCriterion != tTestConfiguration.mConfiguration.mCriteria.cend());
    return criteria::library::SharedLibraryVectorCriterion{tTestConfiguration, *tVectorCriterion, {}};
}

[[nodiscard]] auto create_n_step_vector(const unsigned int aSize) -> linear_algebra::DynamicVector<double>
{
    std::vector<double> tVector;
    tVector.reserve(aSize);
    const double tStep = 1.0 / aSize;
    const double tStart = 0;
    std::generate_n(std::back_inserter(tVector), aSize,
                    [tStart, tStep, tAN = 0]() mutable
                    {
                        tAN++;
                        return tStart + tStep * tAN;
                    });
    return linear_algebra::DynamicVector<double>(std::move(tVector));
}

}  // namespace

TEST_F(SharedLibraryVectorConstraintCoutFixture, SharedLibraryCallValue)
{
    {
        const auto tMeshFilePath = std::filesystem::path{"mesh.exo"};
        constexpr auto tMeshCommand =
            third_party_integration::stk_io::CommandGenerator{{3, 1, 1}, {0, 0, 0}, {3, 1, 1}};
        third_party_integration::stk_io::write_mesh(tMeshFilePath, tMeshCommand);

        const auto tSharedLibrary = test_shared_library_criterion();
        const auto tMasses = tSharedLibrary.value(analysis::AnalysisDomainMesh{tMeshFilePath, {}}).stdVector();
        const auto tExpectedNumberOfElements = tMeshCommand.numberOfElements();
        const auto tExpected = std::vector<double>(tExpectedNumberOfElements, test_mass_criteria::kDensity);
        constexpr auto tTolerance = 1e-15;
        test_utilities::expect_container_entries_near(tExpected, tMasses, tTolerance,
                                                      TEST_CONTEXT("Vector mass density value"));
    }
    const auto tExpectedOutput = std::string{"Unloading mass shared library\nYes, startup was called\n"};
    checkRankZeroCoutStringStream(tExpectedOutput, TEST_CONTEXT("Parallel mass app load/unload text"));
}

TEST_F(SharedLibraryVectorConstraintMeshFixture, MakeCriterionAndCallValue)
{
    const auto [tTempDirectory, tValidatedInput] = integration_tests::utilities::setup_mass_app_for_test(mMeshFilePath);

    const auto tCheckMassDensities =
        [this](const auto& aCriterionFunction, const double aOffset, const test_utilities::TestContext& aTestContext)
    {
        const auto tMasses = aCriterionFunction.template evaluate<core::evaluation::kFunction>(
            analysis::AnalysisDomainMesh{mMeshFilePath, {}});

        const auto tExpected = std::vector<double>(mExpectedNumberOfElements, test_mass_criteria::kDensity - aOffset);
        constexpr auto tTolerance = 1e-15;
        test_utilities::expect_container_entries_near(tExpected, tMasses.stdVector(), tTolerance, aTestContext);
    };

    {
        const auto tCriterion = criteria::library::make_criterion_function<criteria::library::VectorCriterionFunction,
                                                                           input_parser::constraint>(
            tValidatedInput.get<components::ComponentType::kConstraint>().rawInput().front());
        constexpr auto tOffset = 0.0;
        tCheckMassDensities(tCriterion.mFunction, tOffset, TEST_CONTEXT("Test with make_criterion_function"));
    }
    {
        const auto tConstraints =
            criteria::library::make_constraints(tValidatedInput.get<components::ComponentType::kConstraint>());

        constexpr auto tExpectedNumberOfConstraints = 1U;
        ASSERT_EQ(tConstraints.size(), tExpectedNumberOfConstraints);

        constexpr auto tOffset = 0.7;
        tCheckMassDensities(tConstraints.front().mConstraintFunction, tOffset,
                            TEST_CONTEXT("Test with make_constraints"));
    }
}

TEST_F(SharedLibraryVectorConstraintMeshFixture, SharedLibraryCallJacobianTimesVector)
{
    const auto tSharedLibrary = test_shared_library_criterion();
    const auto tDirection = create_n_step_vector(mExpectedNumberOfElements);
    const auto tJacobianTimesVector =
        tSharedLibrary.rowVectorTimesJacobian(analysis::AnalysisDomainMesh{mMeshFilePath, {}}, tDirection).stdVector();

    const auto tExpected = std::vector<double>(tDirection.stdVector().size(), 0.0);
    EXPECT_EQ(tJacobianTimesVector, tExpected);
}

TEST_F(SharedLibraryVectorConstraintMeshFixture, SharedLibraryCallAdjointJacobianTimesVector)
{
    const auto tSharedLibrary = test_shared_library_criterion();
    const auto tDual = create_n_step_vector(mExpectedNumberOfElements);
    const auto tDensityField = analysis::AnalysisDomainMesh::ScalarFieldVector(mCommandGenerator.numberOfNodes());
    const auto tAdjointJacobianTimesDual =
        tSharedLibrary
            .rowVectorTimesAdjointJacobian(analysis::AnalysisDomainMesh{mMeshFilePath, {{1U, tDensityField}}}, tDual)
            .stdVector();

    const auto tExpected = std::vector<double>(mCommandGenerator.numberOfNodes(), 0.0);
    EXPECT_EQ(tAdjointJacobianTimesDual, tExpected);
}

}  // namespace plato::integration_tests::serial
