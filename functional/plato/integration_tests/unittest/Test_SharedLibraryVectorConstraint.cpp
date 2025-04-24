#include <gtest/gtest.h>

#include <filesystem>
#include <iterator>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/criteria/extension/SharedLibraryVectorCriterion.hpp"
#include "plato/criteria/library/ConstraintFactory.hpp"
#include "plato/criteria/library/CriterionFactory.hpp"
#include "plato/criteria/library/test_utilities/ExampleInputBlocks.hpp"
#include "plato/integration_tests/test_mass_criteria/MassConstraintVectorInterface.hpp"
#include "plato/integration_tests/utilities/AppConfigurationTestUtilities.hpp"
#include "plato/integration_tests/utilities/MassAppTestUtilities.hpp"
#include "plato/test_utilities/Containers.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"

namespace plato::integration_tests::serial
{
using third_party_integration::stk_io::test_utilities::OneBlock3x1x1HexMesh;
namespace
{
[[nodiscard]] auto test_shared_library_criterion() -> criteria::extension::SharedLibraryVectorCriterion
{
    const auto tTestConfiguration = utilities::test_app_configuration(utilities::mass_app_library_file_name());
    const auto tVectorCriterion = std::find_if(tTestConfiguration.mConfiguration.mCriteria.cbegin(),
                                               tTestConfiguration.mConfiguration.mCriteria.cend(),
                                               [](const auto& tCriterion) { return !tCriterion.mIsScalar; });
    assert(tVectorCriterion != tTestConfiguration.mConfiguration.mCriteria.cend());
    return criteria::extension::SharedLibraryVectorCriterion{tTestConfiguration, *tVectorCriterion, {}};
}

[[nodiscard]] auto create_n_step_vector(const unsigned int aSize) -> linear_algebra::DynamicVector<double>
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
    return linear_algebra::DynamicVector<double>(std::move(tVector));
}

}  // namespace

TEST_F(OneBlock3x1x1HexMesh, SharedLibraryCallValue)
{
    const auto tSharedLibrary = test_shared_library_criterion();
    const auto tMasses = tSharedLibrary.value(analysis::AnalysisDomainMesh{mMeshFilePath, {}}).stdVector();

    const auto tExpected = std::vector<double>(mExpectedNumberOfElements, test_mass_criteria::kDensity);
    constexpr auto tTolerance = 1e-15;
    test_utilities::expect_container_entries_near(tExpected, tMasses, tTolerance,
                                                  TEST_CONTEXT("Vector mass density value"));
}

TEST_F(OneBlock3x1x1HexMesh, MakeCriterionAndCallValue)
{
    const auto [tTempDirector, tValidatedInput] = integration_tests::utilities::setup_mass_app_for_test(mMeshFilePath);

    const auto tCheckMassDensities =
        [this](const auto& aCriterionFunction, const test_utilities::TestContext& aTestContext)
    {
        const auto tMasses = aCriterionFunction.template evaluate<core::evaluation::kFunction>(
            analysis::AnalysisDomainMesh{mMeshFilePath, {}});

        const auto tExpected = std::vector<double>(mExpectedNumberOfElements, test_mass_criteria::kDensity);
        constexpr auto tTolerance = 1e-15;
        test_utilities::expect_container_entries_near(tExpected, tMasses.stdVector(), tTolerance, aTestContext);
    };

    {
        const auto tCriterion = criteria::library::make_criterion_function<criteria::library::VectorCriterionFunction,
                                                                           input_parser::constraint>(
            tValidatedInput.get<input_parser::ComponentType::kConstraint>().rawInput().front());
        tCheckMassDensities(tCriterion, TEST_CONTEXT("Test with make_criterion_function"));
    }
    {
        const auto tConstraints =
            criteria::library::make_constraints(tValidatedInput.get<input_parser::ComponentType::kConstraint>());

        constexpr auto tExpectedNumberOfConstraints = 1U;
        ASSERT_EQ(tConstraints.size(), tExpectedNumberOfConstraints);

        tCheckMassDensities(tConstraints.front().mConstraintFunction, TEST_CONTEXT("Test with make_constraints"));
    }
}

TEST_F(OneBlock3x1x1HexMesh, SharedLibraryCallJacobianTimesVector)
{
    const auto tSharedLibrary = test_shared_library_criterion();
    const auto tDirection = create_n_step_vector(mExpectedNumberOfElements);
    const auto tJacobianTimesVector =
        tSharedLibrary.rowVectorTimesJacobian(analysis::AnalysisDomainMesh{mMeshFilePath, {}}, tDirection).stdVector();

    const auto tExpected = std::vector<double>(tDirection.stdVector().size(), 0.0);
    EXPECT_EQ(tJacobianTimesVector, tExpected);
}

TEST_F(OneBlock3x1x1HexMesh, SharedLibraryCallAdjointJacobianTimesVector)
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
