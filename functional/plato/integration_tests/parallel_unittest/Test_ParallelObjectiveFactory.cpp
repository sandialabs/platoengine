#include <gtest/gtest.h>

#include <filesystem>
#include <iterator>

#include "plato/criteria/library/ConstraintInputBlock.hpp"
#include "plato/criteria/library/ObjectiveFactory.hpp"
#include "plato/criteria/library/ObjectiveInputBlock.hpp"
#include "plato/filter/extension/IdentityFilter.hpp"
#include "plato/geometry/extension/BrickShapeGeometry.hpp"
#include "plato/geometry/extension/DensityTopology.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/integration_tests/utilities/CheckProcessorsMatchObjectives.hpp"
#include "plato/integration_tests/utilities/InputGeneration.hpp"
#include "plato/integration_tests/utilities/MassAppTestUtilities.hpp"
#include "plato/integration_tests/utilities/ValidInputTestFixture.hpp"
#include "plato/process_manager/extension/ROLOptimization.hpp"
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

constexpr auto kNumRanks = int{4};

[[nodiscard]] auto create_one_objective_test_input() -> input_validation::ValidatedInput
{
    namespace pftu = plato::test_utilities;

    // Input for the actual test
    const std::string tObjectiveInput =
        R"(
          begin new_objective test1
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

void test_parallel_mass_evaluation(const unsigned int aNumGroups, const test_utilities::TestContext& aTestContext)
{
    constexpr auto tMassAppName = std::string_view{"test-mass-app"};
    const auto tComm = boost::mpi::communicator{};
    const auto tConfigurationTempDirectory = utilities::register_test_mass_app(tMassAppName, tComm);

    const auto tObjective = input_parser::new_objective{/*.name=*/std::string{"test_1"},
                                                        /*.active=*/true,
                                                        /*.app=*/input_parser::AppName{std::string{tMassAppName}},
                                                        /*.criterion=*/input_parser::CriterionName{"mass"},
                                                        /*.number_of_processors=*/kNumRanks / aNumGroups,
                                                        /*.input_files=*/boost::none,
                                                        /*.aggregation_weight=*/1.0};

    auto tInput = geometry::extension::create_valid_brick_shape_geometry_input() |
                  process_manager::extension::create_valid_example_rol_optimization_input();
    for ([[maybe_unused]] const auto tIndex : plato::utilities::IndexRange{aNumGroups})
    {
        tInput = tInput | tObjective;
    }

    const auto tValidInput = input_validation::make_validated_input(tInput).value();

    const auto tObjectiveFunction = criteria::library::make_aggregate_objective_function(
        tValidInput.get<input_parser::ComponentType::kObjective>());
    const auto tMeshFileName = tInput.get<input_parser::brick_shape_geometry>().front().mesh_name.value().mToken;
    const auto tGeometry =
        geometry::extension::make_brick_shape_geometry(geometry::extension::BrickShapeGeometry{tMeshFileName});

    const auto tControls = test_brick_controls();
    ASSERT_EQ(tControls.size(), 6u) << aTestContext;
    const auto tExpectedValue = tControls[3] * tControls[4] * tControls[5] * aNumGroups;
    const auto tResult = tObjectiveFunction.evaluate<core::evaluation::kFunction>(
        tGeometry.evaluate<core::evaluation::kFunction>(tControls));
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
    EXPECT_THROW([[maybe_unused]] const auto tData = create_one_objective_test_input(), plato::utilities::Exception);
}

TEST_F(ObjectiveFactoryParallelTestFixture, NumberOfProcessors)
{
    namespace pitu = plato::integration_tests::utilities;
    auto tInputBase = parsedInput();
    tInputBase.template get<input_parser::ComponentType::kObjective>().clear();
    {
        // Set number_of_processors to 4
        auto tObjective = criteria::library::create_valid_example_objective_input();
        tObjective.number_of_processors = static_cast<unsigned int>(kNumRanks);
        const auto tInput = tInputBase | tObjective;
        const auto tValidObjectivesInput = input_validation::make_validated_input(tInput)
                                               .value()
                                               .template get<input_parser::ComponentType::kObjective>();
        pitu::check_processors_match_objectives(
            criteria::library::number_of_processors_per_objective(tValidObjectivesInput), tValidObjectivesInput,
            TEST_CONTEXT("number_of_processors = 4"));
    }
    {
        // Add another objective with 1 processor
        auto tObjective = criteria::library::create_valid_example_objective_input();
        tObjective.number_of_processors = static_cast<unsigned int>(kNumRanks) - 1U;
        const auto tInput = tInputBase | tObjective | criteria::library::create_valid_example_objective_input();
        const auto tValidObjectivesInput = input_validation::make_validated_input(tInput)
                                               .value()
                                               .template get<input_parser::ComponentType::kObjective>();
        pitu::check_processors_match_objectives(
            criteria::library::number_of_processors_per_objective(tValidObjectivesInput), tValidObjectivesInput,
            TEST_CONTEXT("Two objectives, 1 and 3 processors"));
    }
    {
        // Deactivate one objective
        auto tObjective1 = criteria::library::create_valid_example_objective_input();
        tObjective1.number_of_processors = static_cast<unsigned int>(kNumRanks);
        auto tObjective2 = criteria::library::create_valid_example_objective_input();
        tObjective2.active = false;
        const auto tInput = tInputBase | tObjective1 | tObjective2;
        const auto tValidObjectivesInput = input_validation::make_validated_input(tInput)
                                               .value()
                                               .template get<input_parser::ComponentType::kObjective>();
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

}  // namespace plato::integration_tests::parallel
