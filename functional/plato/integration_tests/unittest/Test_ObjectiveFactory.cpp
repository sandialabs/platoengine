#include <gtest/gtest.h>

#include "plato/criteria/library/ObjectiveFactory.hpp"
#include "plato/criteria/library/ObjectiveInputBlock.hpp"
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
struct ObjectiveFactoryTestFixture : public integration_tests::utilities::ValidInputTestFixture
{
};

auto create_two_objective_test_input() -> input_validation::ValidatedInput
{
    namespace pftu = plato::test_utilities;

    // Input for the actual test
    const std::string tInput =
        R"(
          begin new_objective test1
            criterion nodal_sum
            aggregation_weight 42.0
          end
          begin new_objective test2
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

    EXPECT_EQ(tData.get<input_parser::ComponentType::kObjective>().rawInput().size(), 2);
    const auto tAggregate =
        criteria::library::detail::make_parallel_aggregate(tData.get<input_parser::ComponentType::kObjective>());
    EXPECT_EQ(tAggregate.size(), 2);
}

TEST_F(ObjectiveFactoryTestFixture, ValidAggregateOneObjective)
{
    namespace pftu = plato::test_utilities;

    const std::string tInput =
        R"(
          begin new_objective test1
            active false
            criterion nodal_sum
            aggregation_weight 42.0
          end
          begin new_objective test2
            active true
            criterion nodal_sum
            aggregation_weight 13.0
          end
       )" +
        test_utilities::create_valid_density_topology_geometry_string() +
        test_utilities::create_valid_identity_filter_string() +
        test_utilities::create_valid_example_rol_optimization_string();

    const auto tData = input_validation::parse_and_validate_string(tInput).value();

    EXPECT_EQ(tData.get<input_parser::ComponentType::kObjective>().rawInput().size(), 2U);
    const auto tAggregate =
        criteria::library::detail::make_parallel_aggregate(tData.get<input_parser::ComponentType::kObjective>());
    EXPECT_EQ(tAggregate.size(), 1U);
}

TEST_F(ObjectiveFactoryTestFixture, NumberOfProcessors)
{
    {
        // Check example, which sets number_of_processors to 1
        const auto tInput = integration_tests::utilities::create_valid_example_input();
        const auto tValidInput = input_validation::make_validated_input(tInput).value();
        const auto& tObjectives = tValidInput.get<input_parser::ComponentType::kObjective>();
        utilities::check_processors_match_objectives(criteria::library::number_of_processors_per_objective(tObjectives),
                                                     tObjectives, TEST_CONTEXT("One processor"));
    }
    {
        auto tInput = integration_tests::utilities::create_valid_example_input();
        tInput.get<input_parser::ComponentType::kObjective>().clear();
        // Set number_of_processors to boost::none, default is 1
        auto tObjectiveInput = criteria::library::create_valid_example_objective_input();
        tObjectiveInput.number_of_processors = boost::none;
        tInput = tInput | tObjectiveInput;

        const auto tValidInput = input_validation::make_validated_input(tInput).value();
        const auto& tObjectives = tValidInput.get<input_parser::ComponentType::kObjective>();
        utilities::check_processors_match_objectives(criteria::library::number_of_processors_per_objective(tObjectives),
                                                     tObjectives, TEST_CONTEXT("Default using boost::none"));
    }
}

}  // namespace plato::integration_tests::serial
