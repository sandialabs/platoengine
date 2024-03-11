#include <gtest/gtest.h>

#include "plato/criteria/library/ObjectiveFactory.hpp"
#include "plato/main/library/ValidatedInput.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/utilities/Exception.hpp"

namespace plato::integration_tests::serial
{
namespace
{
main::library::ValidatedInput create_two_objective_test_input()
{
    namespace pftu = plato::test_utilities;

    // Input for the actual test
    const std::string tObjectiveInput =
        R"(
          begin objective test1
            app nodal_sum
            aggregation_weight 42.0
          end
          begin objective test2
            active true
            app nodal_sum
            aggregation_weight 13.0
          end
       )";
    // Other inputs to make sure we have valid input
    const std::string tGeometryInput = pftu::create_valid_density_topology_geometry_string();
    const std::string tOptimizerInput = pftu::create_valid_example_optimization_parameters_string();

    return main::library::parse_and_validate(tObjectiveInput + tGeometryInput + tOptimizerInput);
}
}  // namespace

TEST(ObjectiveFactory, ValidAggregate)
{
    const main::library::ValidatedInput tData = create_two_objective_test_input();

    EXPECT_EQ(tData.objectives().rawInput().size(), 2);
    const core::Aggregate tAggregate = criteria::library::detail::make_aggregate(tData.objectives());
    EXPECT_EQ(tAggregate.size(), 2);

    const std::vector tExpected = {42.0, 13.0};
    EXPECT_EQ(tAggregate.weights(), tExpected);
}

TEST(ObjectiveFactory, ValidParallelAggregate)
{
    const main::library::ValidatedInput tData = create_two_objective_test_input();

    EXPECT_EQ(tData.objectives().rawInput().size(), 2);
    const core::ParallelAggregate tAggregate = criteria::library::detail::make_parallel_aggregate(tData.objectives());
    EXPECT_EQ(tAggregate.size(), 2);
}

TEST(ObjectiveFactory, ValidAggregateOneObjective)
{
    namespace pftu = plato::test_utilities;

    // Input for the actual test
    const std::string tObjectiveInput =
        R"(
          begin objective test1
            active false
            app nodal_sum
            aggregation_weight 42.0
          end
          begin objective test2
            active true
            app nodal_sum
            aggregation_weight 13.0
          end
       )";
    // Other inputs to make sure we have valid input
    const std::string tGeometryInput = pftu::create_valid_density_topology_geometry_string();
    const std::string tOptimizerInput = pftu::create_valid_example_optimization_parameters_string();

    const main::library::ValidatedInput tData =
        main::library::parse_and_validate(tObjectiveInput + tGeometryInput + tOptimizerInput);

    EXPECT_EQ(tData.objectives().rawInput().size(), 2);
    const core::Aggregate tAggregate = criteria::library::detail::make_aggregate(tData.objectives());
    EXPECT_EQ(tAggregate.size(), 1);

    const std::vector tExpected = {13.0};
    EXPECT_EQ(tAggregate.weights(), tExpected);
}
}  // namespace plato::integration_tests::serial
