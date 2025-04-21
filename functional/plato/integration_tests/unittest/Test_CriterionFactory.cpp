#include <gtest/gtest.h>

#include <boost/optional/optional_io.hpp>

#include "plato/criteria/library/ConstraintInputBlock.hpp"
#include "plato/criteria/library/CriterionFactory.hpp"
#include "plato/criteria/library/ObjectiveInputBlock.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/integration_tests/utilities/ValidInputTestFixture.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::integration_tests::serial
{
namespace
{
struct CriterionFactoryTestFixture : public utilities::ValidInputTestFixture
{
};
}  // namespace

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

TEST_F(CriterionFactoryTestFixture, ValidObjective)
{
    const auto tData = input_validation::make_validated_input(parsedInput()).value();

    ASSERT_EQ(tData.get<input_parser::ComponentType::kObjective>().rawInput().size(), 1);
    EXPECT_NO_THROW(
        auto tFunction =
            (criteria::library::make_criterion_function<criteria::library::CriterionFunction, input_parser::objective>(
                tData.template get<input_parser::ComponentType::kObjective>().rawInput().front())));
}

TEST_F(CriterionFactoryTestFixture, ValidConstraint)
{
    const auto tData = input_validation::make_validated_input(parsedInput()).value();

    ASSERT_EQ(tData.get<input_parser::ComponentType::kObjective>().rawInput().size(), 1);
    EXPECT_NO_THROW(
        auto tFunction =
            (criteria::library::make_criterion_function<criteria::library::CriterionFunction, input_parser::constraint>(
                tData.template get<input_parser::ComponentType::kConstraint>().rawInput().front())));
}

TEST_F(CriterionFactoryTestFixture, ConvertObjectiveInput)
{
    const std::string tInput =
        R"(
          begin objective test
            active true
            criterion nodal_sum
            number_of_processors 1
            input_files test-input.inp
            aggregation_weight 42.0
          end
       )" +
        test_utilities::create_valid_density_topology_geometry_string() +
        // test_utilities::create_valid_identity_filter_string() +
        test_utilities::create_valid_identity_filter_string() +
        test_utilities::create_valid_example_rol_optimization_string();

    const auto tData = input_validation::parse_and_validate_string(tInput).value();
    ASSERT_EQ(tData.get<input_parser::ComponentType::kObjective>().rawInput().size(), 1U);

    const auto tObjective = input_validation::get_input_block<input_parser::objective>(
        tData.get<input_parser::ComponentType::kObjective>().rawInput().front());

    const auto tCriterionInput = criteria::library::to_criterion_input(tObjective);

    EXPECT_EQ(tObjective.number_of_processors, tCriterionInput.mNumberOfProcessors);
    ASSERT_TRUE(tObjective.input_files.has_value());
    ASSERT_EQ(tObjective.input_files->mList.size(), tCriterionInput.mInputFiles.mList.size());
    for (const auto& [tObjectiveInputFile, tCriterionInputFile] :
         plato::utilities::Zip{tObjective.input_files->mList, tCriterionInput.mInputFiles.mList})
    {
        EXPECT_EQ(tObjectiveInputFile, tCriterionInputFile);
    }
}

TEST_F(CriterionFactoryTestFixture, ConvertConstraintInput)
{
    const auto tData = input_validation::parse_and_validate_string(create_valid_example_input_string()).value();
    const auto tAllConstraints = tData.get<input_parser::ComponentType::kConstraint>().rawInput();
    ASSERT_EQ(tAllConstraints.size(), 1U);
    const auto& tValidatedConstraint = tAllConstraints.front();
    const auto& tConstraint = input_validation::get_input_block<input_parser::constraint>(tValidatedConstraint);
    const auto tCriterionInput = criteria::library::to_criterion_input(tConstraint);

    EXPECT_EQ(tConstraint.number_of_processors, tCriterionInput.mNumberOfProcessors);
    ASSERT_TRUE(tConstraint.input_files.has_value());
    ASSERT_EQ(tConstraint.input_files->mList.size(), tCriterionInput.mInputFiles.mList.size());
    for (const auto& [tConstraintInputFile, tCriterionInputFile] :
         plato::utilities::Zip{tConstraint.input_files->mList, tCriterionInput.mInputFiles.mList})
    {
        EXPECT_EQ(tConstraintInputFile, tCriterionInputFile);
    }
}
}  // namespace plato::integration_tests::serial
