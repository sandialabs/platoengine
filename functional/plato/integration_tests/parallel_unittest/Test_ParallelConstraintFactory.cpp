#include <gtest/gtest.h>

#include "plato/criteria/library/ConstraintFactory.hpp"
#include "plato/criteria/library/ConstraintInputBlock.hpp"
#include "plato/criteria/library/test_utilities/ExampleInputBlocks.hpp"
#include "plato/geometry/extension/BrickShapeGeometry.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/integration_tests/utilities/BrickShapeHelpers.hpp"
#include "plato/integration_tests/utilities/MassAppTestUtilities.hpp"
#include "plato/integration_tests/utilities/ValidInputTestFixture.hpp"

namespace plato::integration_tests::parallel_unittest
{
namespace
{
using ParallelConstraintFactoryFixture = utilities::ValidInputTestFixture;
}

TEST_F(ParallelConstraintFactoryFixture, OneConstraint)
{
    const auto tCommunicator = boost::mpi::communicator{};
    constexpr auto kMassAppName = std::string_view{"test-mass-app"};
    const auto tConfigurationTempDirectory = utilities::register_test_mass_app(kMassAppName, tCommunicator);

    auto tInput = parsedInput();
    tInput.get<components::ComponentType::kConstraint>().clear();

    const auto tConstraint = input_parser::constraint{/*.name=*/std::string{"parallel-mass"},
                                                      /*.active=*/true,
                                                      /*.app=*/input_parser::AppName{std::string{kMassAppName}},
                                                      /*.criterion=*/input_parser::CriterionName{"mass"},
                                                      /*.number_of_processors=*/3U,
                                                      /*.input_files=*/boost::none,
                                                      /*.constraint_value_list=*/boost::none,
                                                      /*.constraint_value=*/0.0,
                                                      /*.is_linear=*/true,
                                                      /*.constraint_type=*/input_parser::ConstraintTypes::kEqualTo};
    const auto tObjective = criteria::library::test_utilities::create_valid_example_objective_input();
    tInput = tInput | tConstraint | tObjective | tObjective | tObjective | tObjective;

    const auto tValidInput = input_validation::make_validated_input(tInput).value();
    const auto tConstraints =
        criteria::library::make_constraints(tValidInput.get<components::ComponentType::kConstraint>());

    ASSERT_EQ(tConstraints.size(), 1U);

    const auto tMeshFileName = std::filesystem::path{"brick-mesh.exo"};
    const auto tGeometry =
        geometry::extension::make_brick_shape_geometry(geometry::extension::BrickShapeGeometry{tMeshFileName});
    const auto tControls = utilities::test_brick_controls();
    const auto tMesh = tGeometry.evaluate<core::evaluation::kFunction>(tControls);

    const auto tExpected = utilities::brick_shape_volume_from_controls(tControls);
    const auto tResult = tConstraints.front().mConstraintFunction.evaluate<core::evaluation::kFunction>(tMesh);
    ASSERT_EQ(tResult.size(), 1U);
    EXPECT_EQ(tResult.stdVector().front(), tExpected);
}

}  // namespace plato::integration_tests::parallel_unittest
