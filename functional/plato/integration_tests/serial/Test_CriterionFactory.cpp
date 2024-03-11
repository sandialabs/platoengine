#include <gtest/gtest.h>

#include <boost/optional/optional_io.hpp>

#include "plato/criteria/library/CriterionFactory.hpp"
#include "plato/main/library/ValidatedInput.hpp"
#include "plato/test_utilities/InputGeneration.hpp"

namespace plato::integration_tests::serial
{
TEST(CriterionFactory, ValidObjective)
{
    namespace pftu = plato::test_utilities;

    const std::string tObjectiveInput = pftu::create_valid_example_objective_string();
    const std::string tGeometryInput = pftu::create_valid_density_topology_geometry_string();
    const std::string tOptimizerInput = pftu::create_valid_example_optimization_parameters_string();
    const main::library::ValidatedInput tData =
        main::library::parse_and_validate(tObjectiveInput + tGeometryInput + tOptimizerInput);

    ASSERT_EQ(tData.objectives().rawInput().size(), 1);
    EXPECT_NO_THROW(auto tFunction = criteria::library::make_criterion_function(tData.objectives().rawInput().front()));
}

TEST(CriterionFactory, ValidConstraint)
{
    namespace pftu = plato::test_utilities;

    const std::string tConstraintInput = pftu::create_valid_example_constraint_string();
    const std::string tObjectiveInput = pftu::create_valid_example_objective_string();
    const std::string tGeometryInput = pftu::create_valid_density_topology_geometry_string();
    const std::string tOptimizerInput = pftu::create_valid_example_optimization_parameters_string();

    const main::library::ValidatedInput tData =
        main::library::parse_and_validate(tConstraintInput + tObjectiveInput + tGeometryInput + tOptimizerInput);

    ASSERT_EQ(tData.constraints().rawInput().size(), 1);
    EXPECT_NO_THROW(auto tFunction =
                        criteria::library::make_criterion_function(tData.constraints().rawInput().front()));
}

TEST(CriterionRegistration, ConvertObjectiveInput)
{
    namespace pftu = plato::test_utilities;

    const std::string tObjectiveInput = pftu::create_valid_example_custom_app_objective_string();
    const std::string tGeometryInput = pftu::create_valid_density_topology_geometry_string();
    const std::string tOptimizerInput = pftu::create_valid_example_optimization_parameters_string();

    const main::library::ValidatedInput tData =
        main::library::parse_and_validate(tObjectiveInput + tGeometryInput + tOptimizerInput);
    ASSERT_EQ(tData.objectives().rawInput().size(), 1);
    const core::ValidatedInputTypeWrapper<input_parser::objective> tValidatedObjective =
        tData.objectives().rawInput().front();

    const criteria::library::CriterionInput tCriterionInput =
        criteria::library::to_criterion_input(tValidatedObjective);
    const input_parser::objective& tObjective = tValidatedObjective.rawInput();

    ASSERT_TRUE(tObjective.shared_library_path.has_value());
    EXPECT_EQ(tObjective.shared_library_path->mName, tCriterionInput.mSharedLibraryPath.mName);
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

    const std::string tConstraintInput = pftu::create_valid_example_constraint_string();
    const std::string tObjectiveInput = pftu::create_valid_example_objective_string();
    const std::string tGeometryInput = pftu::create_valid_density_topology_geometry_string();
    const std::string tOptimizerInput = pftu::create_valid_example_optimization_parameters_string();

    const main::library::ValidatedInput tData =
        main::library::parse_and_validate(tConstraintInput + tObjectiveInput + tGeometryInput + tOptimizerInput);
    ASSERT_EQ(tData.constraints().rawInput().size(), 1);
    const core::ValidatedInputTypeWrapper<input_parser::constraint> tValidatedConstraint =
        tData.constraints().rawInput().front();

    const criteria::library::CriterionInput tCriterionInput =
        criteria::library::to_criterion_input(tValidatedConstraint);
    const input_parser::constraint& tConstraint = tValidatedConstraint.rawInput();

    EXPECT_TRUE(tCriterionInput.mSharedLibraryPath.mName.empty());
    EXPECT_EQ(tConstraint.number_of_processors, tCriterionInput.mNumberOfProcessors);
    ASSERT_TRUE(tConstraint.input_files.has_value());
    ASSERT_EQ(tConstraint.input_files->mList.size(), tCriterionInput.mInputFiles.mList.size());
    for (std::size_t tIndex = 0; tIndex < tConstraint.input_files->mList.size(); ++tIndex)
    {
        EXPECT_EQ(tConstraint.input_files->mList[tIndex], tCriterionInput.mInputFiles.mList[tIndex]);
    }
}
}  // namespace plato::integration_tests::serial
