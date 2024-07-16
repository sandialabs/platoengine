#include <gtest/gtest.h>

#include "plato/criteria/library/ObjectiveFactory.hpp"
#include "plato/integration_tests/utilities/CheckProcessorsMatchObjectives.hpp"
#include "plato/process_manager/library/ValidatedInput.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/utilities/Exception.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::integration_tests::serial
{
namespace
{
process_manager::library::ValidatedInput create_two_objective_test_input()
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

    return process_manager::library::parse_and_validate(tInput);
}
}  // namespace

TEST(ObjectiveFactory, ValidParallelAggregateTwoObjectives)
{
    const process_manager::library::ValidatedInput tData = create_two_objective_test_input();

    EXPECT_EQ(tData.objectives().rawInput().size(), 2);
    const core::ParallelAggregate tAggregate = criteria::library::detail::make_parallel_aggregate(tData.objectives());
    EXPECT_EQ(tAggregate.size(), 2);
}

TEST(ObjectiveFactory, ValidAggregateOneObjective)
{
    namespace pftu = plato::test_utilities;

    // Input for the actual test
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

    const process_manager::library::ValidatedInput tData = process_manager::library::parse_and_validate(tInput);

    EXPECT_EQ(tData.objectives().rawInput().size(), 2);
    const auto tAggregate = criteria::library::detail::make_parallel_aggregate(tData.objectives());
    EXPECT_EQ(tAggregate.size(), 1);
}

TEST(ObjectiveFactory, NumberOfProcessors)
{
    auto tInput = test_utilities::create_valid_example_input();
    {
        // Check example, which sets number_of_processors to 1
        auto tValidInput = process_manager::library::make_validated_input(tInput);
        utilities::check_processors_match_objectives(
            criteria::library::number_of_processors_per_objective(tValidInput.objectives()), tValidInput.objectives(),
            TEST_CONTEXT("One processor"));
    }
    {
        // Set number_of_processors to boost::none, default is 1
        tInput.mObjectives.front().number_of_processors = boost::none;
        auto tValidInput = process_manager::library::make_validated_input(tInput);
        utilities::check_processors_match_objectives(
            criteria::library::number_of_processors_per_objective(tValidInput.objectives()), tValidInput.objectives(),
            TEST_CONTEXT("Default using boost::none"));
    }
}

}  // namespace plato::integration_tests::serial
