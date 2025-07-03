#include <gtest/gtest.h>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/criteria/library/ObjectiveFactory.hpp"
#include "plato/criteria/library/test_utilities/ExampleInputBlocks.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/integration_tests/utilities/CheckProcessorsMatchObjectives.hpp"
#include "plato/integration_tests/utilities/InputGeneration.hpp"
#include "plato/integration_tests/utilities/ValidInputTestFixture.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/utilities/Exception.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::integration_tests::serial
{
namespace
{
constexpr auto kLinearFunctionName = std::string_view{"linear_test_function"};

auto make_linear_test_function() -> criteria::library::CriterionFunction
{
    return core::make_function_with_first_derivative([](const analysis::AnalysisDomainMesh& aMesh)
                                                     { return aMesh.mBlockScalarField.begin()->second.front().mValue; },
                                                     [](const analysis::AnalysisDomainMesh&)
                                                     { return linear_algebra::DynamicVector<double>{1.0}; });
}

using Registration = criteria::library::CriterionRegistration<criteria::library::Parallelization::kSerial,
                                                              criteria::library::FunctionDimension::kScalar>;

[[maybe_unused]] static auto kNodalSumRegistration =
    Registration{criteria::library::builtin_criterion_registration_name(kLinearFunctionName),
                 [](const criteria::library::CriterionInput&) { return make_linear_test_function(); }};

struct ObjectiveFactoryTestFixture : public integration_tests::utilities::ValidInputTestFixture
{
};

auto create_two_objective_test_input() -> input_validation::ValidatedInput
{
    namespace pftu = plato::test_utilities;

    // Input for the actual test
    const std::string tInput =
        R"(
          begin objective test1
            criterion nodal_sum
            aggregation_weight 42.0
          end
          begin objective test2
            active true
            criterion nodal_sum
            aggregation_weight 13.0
          end
       )" +
        test_utilities::create_valid_density_topology_geometry_string() +
        test_utilities::create_valid_identity_filter_string() +
        test_utilities::create_valid_example_rol_optimization_string();

    return input_validation::parse_and_validate_string(tInput).value();
}
}  // namespace

TEST_F(ObjectiveFactoryTestFixture, ValidParallelAggregateTwoObjectives)
{
    const auto tData = create_two_objective_test_input();

    EXPECT_EQ(tData.get<components::ComponentType::kObjective>().rawInput().size(), 2);
    const auto tAggregate =
        criteria::library::detail::make_parallel_aggregate(tData.get<components::ComponentType::kObjective>());
    EXPECT_EQ(tAggregate.size(), 2);
}

TEST_F(ObjectiveFactoryTestFixture, ValidAggregateOneObjective)
{
    namespace pftu = plato::test_utilities;

    const std::string tInput =
        R"(
          begin objective test1
            active false
            criterion nodal_sum
            aggregation_weight 42.0
          end
          begin objective test2
            active true
            criterion nodal_sum
            aggregation_weight 13.0
          end
       )" +
        test_utilities::create_valid_density_topology_geometry_string() +
        test_utilities::create_valid_identity_filter_string() +
        test_utilities::create_valid_example_rol_optimization_string();

    const auto tData = input_validation::parse_and_validate_string(tInput).value();

    EXPECT_EQ(tData.get<components::ComponentType::kObjective>().rawInput().size(), 2U);
    const auto tAggregate =
        criteria::library::detail::make_parallel_aggregate(tData.get<components::ComponentType::kObjective>());
    EXPECT_EQ(tAggregate.size(), 1U);
}

TEST_F(ObjectiveFactoryTestFixture, NumberOfProcessors)
{
    {
        // Check example, which sets number_of_processors to 1
        const auto tInput = integration_tests::utilities::create_valid_example_input();
        const auto tValidInput = input_validation::make_validated_input(tInput).value();
        const auto& tObjectives = tValidInput.get<components::ComponentType::kObjective>();
        utilities::check_processors_match_objectives(criteria::library::number_of_processors_per_objective(tObjectives),
                                                     tObjectives, TEST_CONTEXT("One processor"));
    }
    {
        auto tInput = integration_tests::utilities::create_valid_example_input();
        tInput.get<components::ComponentType::kObjective>().clear();
        // Set number_of_processors to boost::none, default is 1
        auto tObjectiveInput = criteria::library::test_utilities::create_valid_example_objective_input();
        tObjectiveInput.number_of_processors = boost::none;
        tInput = tInput | tObjectiveInput;

        const auto tValidInput = input_validation::make_validated_input(tInput).value();
        const auto& tObjectives = tValidInput.get<components::ComponentType::kObjective>();
        utilities::check_processors_match_objectives(criteria::library::number_of_processors_per_objective(tObjectives),
                                                     tObjectives, TEST_CONTEXT("Default using boost::none"));
    }
}

TEST_F(ObjectiveFactoryTestFixture, ObjectiveGoal)
{
    constexpr auto tX = 21.0;

    const auto tTestFunction = [](criteria::library::ObjectiveGoal aObjectiveGoal)
    {
        const auto tObjectiveInput =
            input_parser::objective{/*.name=*/std::string{"test"},
                                    /*.active=*/true,
                                    /*.app=*/input_parser::AppName{"platoengine"},
                                    /*.criterion=*/input_parser::CriterionName{std::string{kLinearFunctionName}},
                                    /*.number_of_processors=*/1U,
                                    /*.input_files=*/plato::input_parser::FileList{},
                                    /*.aggregation_weight=*/2.0,
                                    /*.objective_goal*/ aObjectiveGoal};
        auto tInput = integration_tests::utilities::create_valid_example_input();
        tInput.get<input_parser::ComponentType::kObjective>().clear();
        tInput = tInput | tObjectiveInput;
        const auto tValidInput = input_validation::make_validated_input(tInput).value();
        const auto& tObjectives = tValidInput.get<input_parser::ComponentType::kObjective>();

        const auto tObjective = criteria::library::make_aggregate_objective_function(tObjectives);

        const auto tMeshField = std::vector{
            analysis::ScalarFieldValue{.mGlobalMeshEntityID = 0U, .mDesignVariableVectorIndex = 0U, .mValue = tX}};
        const auto tMesh =
            analysis::AnalysisDomainMesh{.mFileName = "mesh.exo", .mBlockScalarField = {{0U, tMeshField}}};
        return std::make_pair(tObjective.template evaluate<core::evaluation::kFunction>(tMesh),
                              tObjective.template evaluate<core::evaluation::kFirstDerivative>(tMesh));
    };

    // Minimize
    {
        const auto [tObjective, tGradient] = tTestFunction(criteria::library::ObjectiveGoal::kMinimize);
        EXPECT_EQ(2.0 * tX, tObjective);
        EXPECT_EQ(2.0, tGradient[0]);
    }
    // Maximize
    {
        const auto [tObjective, tGradient] = tTestFunction(criteria::library::ObjectiveGoal::kMaximize);
        EXPECT_EQ(-2.0 * tX, tObjective);
        EXPECT_EQ(-2.0, tGradient[0]);
    }
}

}  // namespace plato::integration_tests::serial
