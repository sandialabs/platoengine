#include <gtest/gtest.h>

#include <boost/optional/optional_io.hpp>

#include "plato/criteria/library/CriterionFactory.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/process_manager/library/ValidatedInput.hpp"
#include "plato/test_utilities/InputGeneration.hpp"

namespace plato::integration_tests::serial
{

namespace
{
[[nodiscard]] std::string create_valid_example_input_string()
{
    return test_utilities::create_valid_example_constraint_string() +
           test_utilities::create_valid_example_objective_string() +
           test_utilities::create_valid_density_topology_geometry_string() +
           test_utilities::create_valid_identity_filter_string() +
           test_utilities::create_valid_example_rol_optimization_string();
}
}  // namespace

TEST(CriterionFactory, ValidObjective)
{
    const auto tData = process_manager::library::make_validated_input(
        test_utilities::create_valid_density_topology_geometry() | test_utilities::create_valid_example_objective() |
        test_utilities::create_valid_example_rol_optimization() | test_utilities::create_valid_identity_filter());

    ASSERT_EQ(tData.objectives().rawInput().size(), 1);
    EXPECT_NO_THROW(auto tFunction = criteria::library::make_criterion_function(tData.objectives().rawInput().front()));
}

TEST(CriterionFactory, ValidConstraint)
{
    const auto tData = process_manager::library::make_validated_input(
        test_utilities::create_valid_density_topology_geometry() | test_utilities::create_valid_example_objective() |
        test_utilities::create_valid_example_constraint() | test_utilities::create_valid_example_rol_optimization() |
        test_utilities::create_valid_identity_filter());

    ASSERT_EQ(tData.constraints().rawInput().size(), 1);
    EXPECT_NO_THROW(auto tFunction =
                        criteria::library::make_criterion_function(tData.constraints().rawInput().front()));
}

TEST(CriterionRegistration, ConvertObjectiveInput)
{
    const std::string tInput =
        R"(
          begin objective test
            active true
            criterion nodal_sum
            number_of_processors 1
            input_files test-input.inp
            aggregation_weight 42.0
            objective_type minimize
          end
       )" +
        test_utilities::create_valid_density_topology_geometry_string() +
        test_utilities::create_valid_identity_filter_string() +
        test_utilities::create_valid_example_rol_optimization_string();

    const process_manager::library::ValidatedInput tData = process_manager::library::parse_and_validate(tInput);
    ASSERT_EQ(tData.objectives().rawInput().size(), 1);
    const core::ValidatedInputTypeWrapper<input_parser::objective> tValidatedObjective =
        tData.objectives().rawInput().front();

    const criteria::library::CriterionInput tCriterionInput =
        criteria::library::to_criterion_input(tValidatedObjective);
    const input_parser::objective& tObjective = tValidatedObjective.rawInput();

    EXPECT_EQ(tObjective.number_of_processors, tCriterionInput.mNumberOfProcessors);
    ASSERT_TRUE(tObjective.input_files.has_value());
    ASSERT_EQ(tObjective.input_files->mList.size(), tCriterionInput.mInputFiles.mList.size());
    for (std::size_t tIndex = 0; tIndex < tObjective.input_files->mList.size(); ++tIndex)
    {
        EXPECT_EQ(tObjective.input_files->mList[tIndex], tCriterionInput.mInputFiles.mList[tIndex]);
    }
}

TEST(CriterionRegistration, ConvertConstraintInput)
{
    namespace pftu = plato::test_utilities;
    const process_manager::library::ValidatedInput tData =
        process_manager::library::parse_and_validate(create_valid_example_input_string());
    ASSERT_EQ(tData.constraints().rawInput().size(), 1);
    const core::ValidatedInputTypeWrapper<input_parser::constraint> tValidatedConstraint =
        tData.constraints().rawInput().front();

    const criteria::library::CriterionInput tCriterionInput =
        criteria::library::to_criterion_input(tValidatedConstraint);
    const input_parser::constraint& tConstraint = tValidatedConstraint.rawInput();

    EXPECT_EQ(tConstraint.number_of_processors, tCriterionInput.mNumberOfProcessors);
    ASSERT_TRUE(tConstraint.input_files.has_value());
    ASSERT_EQ(tConstraint.input_files->mList.size(), tCriterionInput.mInputFiles.mList.size());
    for (std::size_t tIndex = 0; tIndex < tConstraint.input_files->mList.size(); ++tIndex)
    {
        EXPECT_EQ(tConstraint.input_files->mList[tIndex], tCriterionInput.mInputFiles.mList[tIndex]);
    }
}
}  // namespace plato::integration_tests::serial
