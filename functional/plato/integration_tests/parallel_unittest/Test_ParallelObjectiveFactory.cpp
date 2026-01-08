#include <gtest/gtest.h>

#include <filesystem>

#include "plato/criteria/library/ObjectiveFactory.hpp"
#include "plato/criteria/library/test_utilities/ExampleInputBlocks.hpp"
#include "plato/geometry/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/integration_tests/utilities/CheckProcessorsMatchObjectives.hpp"
#include "plato/integration_tests/utilities/MassAppTestUtilities.hpp"
#include "plato/integration_tests/utilities/ValidInputTestFixture.hpp"
#include "plato/process_manager/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/utilities/Exception.hpp"
#include "plato/utilities/IndexRange.hpp"

namespace plato::integration_tests::parallel
{
namespace
{
struct ObjectiveFactoryParallelTestFixture : public utilities::ValidInputTestFixture
{
};

constexpr auto kMassAppName = std::string_view{"test-mass-app"};

constexpr auto kNumRanks = int{4};

struct ObjectiveCopyTestData
{
    unsigned int mNumObjectiveCopies;
    unsigned int mNumProcessorsPerObjective;
};

auto base_mass_objective() -> input_parser::objective
{
    auto tObjective = criteria::library::test_utilities::create_valid_example_objective_input();
    tObjective.app = input_parser::AppName{std::string{kMassAppName}};
    tObjective.criterion = input_parser::CriterionName{"mass"};
    return tObjective;
}

[[nodiscard]] auto create_one_objective_test_input() -> input_validation::ValidatedInput
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
    const std::string tIdentityFilterInput = pftu::create_valid_identity_filter_string();

    return input_validation::parse_and_validate_string(tObjectiveInput + tGeometryInput + tIdentityFilterInput +
                                                       tOptimizerInput)
        .value();
}

linear_algebra::DynamicVector<double> test_brick_controls()
{
    constexpr auto tX = double{2.0};
    constexpr auto tY = double{4.0};
    constexpr auto tZ = double{6.0};
    constexpr auto tCenterCoordinate = double{0.0};
    return linear_algebra::DynamicVector<double>{tCenterCoordinate, tCenterCoordinate, tCenterCoordinate, tX, tY, tZ};
}

double brick_shape_volume_from_controls(const linear_algebra::DynamicVector<double>& aControls)
{
    const auto tDimensionX = aControls[3];
    const auto tDimensionY = aControls[4];
    const auto tDimensionZ = aControls[5];
    return tDimensionX * tDimensionY * tDimensionZ;
}

auto brick_shape_geometry_from_input(const input_parser::ParsedInput& aInput) -> geometry::library::GeometryFunction
{
    const auto tMeshFileName = aInput.get<input_parser::brick_shape_geometry>().front().mesh_name.value().mToken;
    return geometry::extension::make_brick_shape_geometry(geometry::extension::BrickShapeGeometry{tMeshFileName});
}

void test_parallel_mass_evaluation(const unsigned int aNumGroups, const test_utilities::TestContext& aTestContext)
{
    const auto tComm = boost::mpi::communicator{};
    const auto tConfigurationTempDirectory = utilities::register_test_mass_app(kMassAppName, tComm);

    const auto tObjective = input_parser::objective{/*.name=*/std::string{"test_1"},
                                                    /*.active=*/true,
                                                    /*.app=*/input_parser::AppName{std::string{kMassAppName}},
                                                    /*.criterion=*/input_parser::CriterionName{"mass"},
                                                    /*.number_of_processors=*/kNumRanks / aNumGroups,
                                                    /*.input_files=*/boost::none,
                                                    /*.aggregation_weight=*/1.0,
                                                    /*.normalize_by_initial_value=*/false,
                                                    /*.objective_goal=*/boost::none};

    auto tInput = geometry::extension::test_utilities::create_valid_brick_shape_geometry_input() |
                  process_manager::extension::test_utilities::create_valid_example_rol_optimization_input();
    for ([[maybe_unused]] const auto tIndex : plato::utilities::IndexRange{aNumGroups})
    {
        tInput = tInput | tObjective;
    }

    const auto tValidInput = input_validation::make_validated_input(tInput).value();

    const auto tGeometry = brick_shape_geometry_from_input(tInput);
    const auto tObjectiveFunction = criteria::library::make_aggregate_objective_function(
        tValidInput.get<components::ComponentType::kObjective>(),
        tGeometry.evaluate<core::evaluation::kFunction>(test_brick_controls()));

    const auto tControls = test_brick_controls();
    ASSERT_EQ(tControls.size(), 6u) << aTestContext;
    const auto tExpectedValue = brick_shape_volume_from_controls(tControls) * aNumGroups;
    const auto tResult = tObjectiveFunction.evaluate<core::evaluation::kFunction>(
        tGeometry.evaluate<core::evaluation::kFunction>(tControls));
    EXPECT_EQ(tResult, tExpectedValue) << aTestContext;
}

void test_aggregate_normalized_objectives(const ObjectiveCopyTestData& aObjectiveCopyTestData,
                                          input_parser::ParsedInput aInput,
                                          input_parser::objective aObjective,
                                          const test_utilities::TestContext& aTestContext)
{
    const auto tGeometry = brick_shape_geometry_from_input(aInput);

    aObjective.number_of_processors = static_cast<unsigned int>(aObjectiveCopyTestData.mNumProcessorsPerObjective);
    for ([[maybe_unused]] const auto tIndex : plato::utilities::IndexRange{aObjectiveCopyTestData.mNumObjectiveCopies})
    {
        aInput = aInput | aObjective;
    }
    const auto tValidObjectivesInput =
        input_validation::make_validated_input(aInput).value().template get<components::ComponentType::kObjective>();

    const auto tInitialDomainMesh = tGeometry.evaluate<core::evaluation::kFunction>(test_brick_controls());
    const auto tAggregateObjectiveFunction =
        criteria::library::make_aggregate_objective_function(tValidObjectivesInput, tInitialDomainMesh);

    const auto tResult = tAggregateObjectiveFunction.evaluate<core::evaluation::kFunction>(tInitialDomainMesh);
    EXPECT_EQ(tResult, aObjectiveCopyTestData.mNumObjectiveCopies) << aTestContext;
}

}  // namespace

TEST(ObjectiveFactory, MPISize)
{
    auto tComm = boost::mpi::communicator{};
    EXPECT_EQ(tComm.size(), kNumRanks);
}

TEST(ObjectiveFactory, InvalidParallelAggregate)
{
    EXPECT_THROW([[maybe_unused]] const auto tData = create_one_objective_test_input(), plato::utilities::Exception);
}

TEST_F(ObjectiveFactoryParallelTestFixture, NumberOfProcessors)
{
    namespace pitu = plato::integration_tests::utilities;
    auto tInputBase = parsedInput();
    tInputBase.template get<components::ComponentType::kObjective>().clear();
    {
        // Set number_of_processors to 4
        auto tObjective = criteria::library::test_utilities::create_valid_example_objective_input();
        tObjective.number_of_processors = static_cast<unsigned int>(kNumRanks);
        const auto tInput = tInputBase | tObjective;
        const auto tValidObjectivesInput = input_validation::make_validated_input(tInput)
                                               .value()
                                               .template get<components::ComponentType::kObjective>();
        pitu::check_processors_match_objectives(
            criteria::library::number_of_processors_per_objective(tValidObjectivesInput), tValidObjectivesInput,
            TEST_CONTEXT("number_of_processors = 4"));
    }
    {
        // Add another objective with 1 processor
        auto tObjective = criteria::library::test_utilities::create_valid_example_objective_input();
        tObjective.number_of_processors = static_cast<unsigned int>(kNumRanks) - 1U;
        const auto tInput =
            tInputBase | tObjective | criteria::library::test_utilities::create_valid_example_objective_input();
        const auto tValidObjectivesInput = input_validation::make_validated_input(tInput)
                                               .value()
                                               .template get<components::ComponentType::kObjective>();
        pitu::check_processors_match_objectives(
            criteria::library::number_of_processors_per_objective(tValidObjectivesInput), tValidObjectivesInput,
            TEST_CONTEXT("Two objectives, 1 and 3 processors"));
    }
    {
        // Deactivate one objective
        auto tObjective1 = criteria::library::test_utilities::create_valid_example_objective_input();
        tObjective1.number_of_processors = static_cast<unsigned int>(kNumRanks);
        auto tObjective2 = criteria::library::test_utilities::create_valid_example_objective_input();
        tObjective2.active = false;
        const auto tInput = tInputBase | tObjective1 | tObjective2;
        const auto tValidObjectivesInput = input_validation::make_validated_input(tInput)
                                               .value()
                                               .template get<components::ComponentType::kObjective>();
        pitu::check_processors_match_objectives(
            criteria::library::number_of_processors_per_objective(tValidObjectivesInput), tValidObjectivesInput,
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

TEST(ObjectiveFactory, EvaluateParallelMassAppWithNormalization)
{
    const auto tComm = boost::mpi::communicator{};
    const auto tConfigurationTempDirectory = utilities::register_test_mass_app(kMassAppName, tComm);

    const auto tInputBase = geometry::extension::test_utilities::create_valid_brick_shape_geometry_input() |
                            process_manager::extension::test_utilities::create_valid_example_rol_optimization_input();

    auto tObjectiveBase = base_mass_objective();
    tObjectiveBase.aggregation_weight = 1.0;
    tObjectiveBase.normalize_by_initial_value = true;
    {
        const ObjectiveCopyTestData tTestData{.mNumObjectiveCopies = 1u, .mNumProcessorsPerObjective = 4u};
        test_aggregate_normalized_objectives(tTestData, tInputBase, tObjectiveBase,
                                             TEST_CONTEXT("1 objective split over the 4 ranks"));
    }
    {
        const ObjectiveCopyTestData tTestData{.mNumObjectiveCopies = 2u, .mNumProcessorsPerObjective = 2u};
        test_aggregate_normalized_objectives(tTestData, tInputBase, tObjectiveBase,
                                             TEST_CONTEXT("2 objectives using 2 ranks each"));
    }
    {
        const ObjectiveCopyTestData tTestData{.mNumObjectiveCopies = 12u, .mNumProcessorsPerObjective = 1u};
        tObjectiveBase.objective_goal = criteria::library::ObjectiveGoal::kMinimizeReciprocal;
        test_aggregate_normalized_objectives(tTestData, tInputBase, tObjectiveBase,
                                             TEST_CONTEXT("12 objectives split over the 4 ranks, minimize reciprocal"));
    }
}

TEST(ObjectiveFactory, ManyInactiveObjectives)
{
    const auto tComm = boost::mpi::communicator{};
    const auto tConfigurationTempDirectory = utilities::register_test_mass_app(kMassAppName, tComm);

    const auto tInputBase = geometry::extension::test_utilities::create_valid_brick_shape_geometry_input() |
                            process_manager::extension::test_utilities::create_valid_example_rol_optimization_input();

    const auto tAppName = input_parser::AppName{std::string{kMassAppName}};
    const auto tCriterionName = input_parser::CriterionName{"mass"};
    const auto tOneRankActiveObjective = input_parser::objective{/*.name=*/std::string{"one_rank_active_objective"},
                                                                 /*.active=*/true,
                                                                 /*.app=*/tAppName,
                                                                 /*.criterion=*/tCriterionName,
                                                                 /*.number_of_processors=*/1U,
                                                                 /*.input_files=*/boost::none,
                                                                 /*.aggregation_weight=*/1.0,
                                                                 /*.normalize_by_initial_value=*/false,
                                                                 /*.objective_goal=*/boost::none};
    const auto tTwoRankActiveObjective = input_parser::objective{/*.name=*/std::string{"two_rank_active_objective"},
                                                                 /*.active=*/true,
                                                                 /*.app=*/tAppName,
                                                                 /*.criterion=*/tCriterionName,
                                                                 /*.number_of_processors=*/2U,
                                                                 /*.input_files=*/boost::none,
                                                                 /*.aggregation_weight=*/1.0,
                                                                 /*.normalize_by_initial_value=*/false,
                                                                 /*.objective_goal=*/boost::none};
    const auto tInactiveObjective = input_parser::objective{/*.name=*/std::string{"inactive_objective"},
                                                            /*.active=*/false,
                                                            /*.app=*/tAppName,
                                                            /*.criterion=*/tCriterionName,
                                                            /*.number_of_processors=*/1U,
                                                            /*.input_files=*/boost::none,
                                                            /*.aggregation_weight=*/1.0,
                                                            /*.normalize_by_initial_value=*/false,
                                                            /*.objective_goal=*/boost::none};
    const auto tInput = tInputBase | tInactiveObjective | tInactiveObjective | tTwoRankActiveObjective |
                        tOneRankActiveObjective | tInactiveObjective | tInactiveObjective | tOneRankActiveObjective;

    const auto tValidInput = input_validation::make_validated_input(tInput).value();
    const auto tGeometry = brick_shape_geometry_from_input(tInput);
    const auto tMesh = tGeometry.evaluate<core::evaluation::kFunction>(test_brick_controls());
    constexpr auto tRegressionValue = 144.0;  // Computed by running the test with no inactive objectives

    const auto tObjectiveFunction = criteria::library::make_aggregate_objective_function(
        tValidInput.get<components::ComponentType::kObjective>(), tMesh);
    const auto tResult = tObjectiveFunction.evaluate<core::evaluation::kFunction>(tMesh);
    EXPECT_EQ(tResult, tRegressionValue);
}

}  // namespace plato::integration_tests::parallel
