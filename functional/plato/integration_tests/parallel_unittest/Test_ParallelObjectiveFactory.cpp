#include <gtest/gtest.h>

#include <filesystem>
#include <iterator>

#include "plato/criteria/library/ObjectiveFactory.hpp"
#include "plato/geometry/extension/BrickShapeGeometry.hpp"
#include "plato/integration_tests/utilities/CheckProcessorsMatchObjectives.hpp"
#include "plato/integration_tests/utilities/MassAppTestUtilities.hpp"
#include "plato/process_manager/library/ValidatedInput.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/utilities/Exception.hpp"

namespace plato::integration_tests::parallel
{
namespace
{
constexpr auto kNumRanks = int{4};

process_manager::library::ValidatedInput create_one_objective_test_input()
{
    namespace pftu = plato::test_utilities;

    // Input for the actual test
    const std::string tObjectiveInput =
        R"(
          begin objective test1
            criterion nodal_sum
            aggregation_weight 42.0
          end
       )";
    // Other inputs to make sure we have valid input
    const std::string tGeometryInput = pftu::create_valid_density_topology_geometry_string();
    const std::string tOptimizerInput = pftu::create_valid_example_rol_optimization_string();

    return process_manager::library::parse_and_validate(tObjectiveInput + tGeometryInput + tOptimizerInput);
}

linear_algebra::DynamicVector<double> test_brick_controls()
{
    constexpr auto tX = double{2.0};
    constexpr auto tY = double{4.0};
    constexpr auto tZ = double{6.0};
    constexpr auto tCenterCoordinate = double{0.0};
    return linear_algebra::DynamicVector<double>{tCenterCoordinate, tCenterCoordinate, tCenterCoordinate, tX, tY, tZ};
}

void test_parallel_mass_evaluation(const unsigned int aNumGroups, const test_utilities::TestContext& aTestContext)
{
    constexpr auto tMassAppName = std::string_view{"test-mass-app"};
    const auto tComm = boost::mpi::communicator{};
    const auto tConfigurationTempDirectory = utilities::register_test_mass_app(tMassAppName, tComm);

    auto tObjective = input_parser::objective{};
    tObjective.number_of_processors = kNumRanks / aNumGroups;
    tObjective.aggregation_weight = 1.0;
    tObjective.app = input_parser::AppName{std::string{tMassAppName}};
    tObjective.criterion = input_parser::CriterionName{"mass"};
    tObjective.name = "test_1";

    auto tInput = input_parser::ParsedInput{};
    std::fill_n(std::back_inserter(tInput.mObjectives), aNumGroups, tObjective);
    tInput.mBrickShapeGeometry = test_utilities::create_valid_brick_shape_geometry();
    tInput.mROLOptimization = test_utilities::create_valid_example_rol_optimization();

    const auto tValidInput = process_manager::library::make_validated_input(tInput);

    const auto tObjectiveFunction = criteria::library::make_aggregate_objective_function(tValidInput.objectives());
    const auto tMeshFileName = tInput.mBrickShapeGeometry->mesh_name.value().mToken;
    const auto tGeometry =
        geometry::extension::make_brick_shape_geometry(geometry::extension::BrickShapeGeometry{tMeshFileName});

    const auto tControls = test_brick_controls();
    ASSERT_EQ(tControls.size(), 6u) << aTestContext;
    const auto tExpectedValue = tControls[3] * tControls[4] * tControls[5] * aNumGroups;
    const auto tResult = tObjectiveFunction.f(tGeometry.f(tControls));
    EXPECT_EQ(tResult, tExpectedValue) << aTestContext;
}

}  // namespace

TEST(ObjectiveFactory, MPISize)
{
    auto tComm = boost::mpi::communicator{};
    EXPECT_EQ(tComm.size(), kNumRanks);
}

TEST(ObjectiveFactory, InvalidParallelAggregate)
{
    EXPECT_THROW(const process_manager::library::ValidatedInput tData = create_one_objective_test_input(),
                 plato::utilities::Exception);
}

TEST(ObjectiveFactory, NumberOfProcessors)
{
    namespace pitu = plato::integration_tests::utilities;
    auto tInput = test_utilities::create_valid_example_input();
    {
        // Set number_of_processors to 4
        tInput.mObjectives.front().number_of_processors = static_cast<unsigned int>(kNumRanks);
        auto tValidInput = process_manager::library::make_validated_input(tInput);
        pitu::check_processors_match_objectives(
            criteria::library::number_of_processors_per_objective(tValidInput.objectives()), tValidInput.objectives(),
            TEST_CONTEXT("number_of_processors = 4"));
    }
    {
        // Add another objective with 1 processor
        tInput.mObjectives.front().number_of_processors = static_cast<unsigned int>(kNumRanks) - 1u;
        tInput.mObjectives.push_back(test_utilities::create_valid_example_objective());
        auto tValidInput = process_manager::library::make_validated_input(tInput);
        pitu::check_processors_match_objectives(
            criteria::library::number_of_processors_per_objective(tValidInput.objectives()), tValidInput.objectives(),
            TEST_CONTEXT("Two objectives, 1 and 3 processors"));
    }
    {
        // Deactivate one objective
        tInput.mObjectives.front().number_of_processors = static_cast<unsigned int>(kNumRanks);
        tInput.mObjectives.back().active = false;
        auto tValidInput = process_manager::library::make_validated_input(tInput);
        pitu::check_processors_match_objectives(
            criteria::library::number_of_processors_per_objective(tValidInput.objectives()), tValidInput.objectives(),
            TEST_CONTEXT("Two objectives, one with active = false"));
    }
}

TEST(ObjectiveFactory, EvaluateParallelMassAppTwoObjectives)
{
    constexpr auto tNumGroups = 2u;
    test_parallel_mass_evaluation(tNumGroups, TEST_CONTEXT("Two objectives"));
}

TEST(ObjectiveFactory, EvaluateParallelMassAppOneObjective)
{
    constexpr auto tNumGroups = 1u;
    test_parallel_mass_evaluation(tNumGroups, TEST_CONTEXT("One objective"));
}

}  // namespace plato::integration_tests::parallel
