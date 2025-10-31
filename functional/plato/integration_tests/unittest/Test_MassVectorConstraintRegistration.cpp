#include <gtest/gtest.h>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/criteria/library/ConstraintInputBlock.hpp"
#include "plato/criteria/library/ObjectiveInputBlock.hpp"
#include "plato/geometry/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/integration_tests/utilities/MassAppTestUtilities.hpp"
#include "plato/process_manager/extension/ConstraintCheck.hpp"
#include "plato/process_manager/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/process_manager/library/ProcessManagerFactory.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"

namespace plato::integration_tests::unittest
{
namespace
{
class MassVectorConstraint : public third_party_integration::stk_io::test_utilities::OneBlock3x1x1HexMesh
{
};

auto create_test_mass_vector_constraint_input(const input_parser::AppName& aMassAppName,
                                              const input_parser::CriterionName& aCriterionName)
    -> input_validation::ValidatedInput
{
    auto tObjective = input_parser::objective{};
    tObjective.number_of_processors = 1U;
    tObjective.aggregation_weight = 1.0;
    tObjective.app = aMassAppName;
    tObjective.criterion = input_parser::CriterionName{"mass"};
    tObjective.name = "test_1";

    auto tConstraint = input_parser::constraint{};
    tConstraint.number_of_processors = 1U;
    tConstraint.app = aMassAppName;
    tConstraint.criterion = aCriterionName;
    tConstraint.name = "test_2";
    tConstraint.constraint_type = input_parser::ConstraintTypes::kGreaterThan;
    tConstraint.constraint_value_list = criteria::library::ConstraintValueList{
        {input_parser::ComponentAndTarget{/*.component=*/input_parser::IdentifierString{"cg_x"}, /*.target=*/42.0},
         input_parser::ComponentAndTarget{/*.component=*/input_parser::IdentifierString{"cg_z"}, /*.target=*/43.0}}};

    const auto tInput = tObjective | tConstraint |
                        geometry::extension::test_utilities::create_valid_brick_shape_geometry_input() |
                        process_manager::extension::test_utilities::create_valid_example_constraint_check_input();
    return input_validation::make_validated_input(tInput).value();
}
}  // namespace

TEST_F(MassVectorConstraint, MassPropertiesVectorConstraintRegisterAndRun)
{
    const auto tAppName = input_parser::AppName{"test-mass-app"};
    auto tConfigurationTempDirectory = utilities::register_test_mass_app(tAppName.mToken, boost::mpi::communicator{});
    const auto tValidatedInput =
        create_test_mass_vector_constraint_input(tAppName, input_parser::CriterionName{"mass-properties"});

    const auto tProcessManager = process_manager::library::make_process_managers(
        tValidatedInput.get<components::ComponentType::kProcessManager>());
    const auto tProcessManagerData = process_manager::library::make_process_manager_data(tValidatedInput);

    ASSERT_EQ(tProcessManagerData.mConstraints.size(), 1U);
    const auto& tConstraint = tProcessManagerData.mConstraints.front();

    // Check constraint input
    const auto tExpectedTargets = std::vector<double>{42.0, 43.0};
    ASSERT_EQ(tConstraint.mConstraintTarget.size(), tExpectedTargets.size());
    EXPECT_EQ(tConstraint.mConstraintTarget.value(tExpectedTargets.size()), tExpectedTargets);

    // Evaluate and check size
    const auto tEvaluationResult = tConstraint.mConstraintFunction.evaluate<0>(analysis::AnalysisDomainMesh{});
    const auto tExpectedResult = std::vector<double>{1.0, 3.0};
    EXPECT_EQ(tEvaluationResult.stdVector(), tExpectedResult);
}

TEST_F(MassVectorConstraint, MassVectorConstraintRegisterAndRun)
{
    const auto [tTempDirectory, tValidatedInput] = integration_tests::utilities::setup_mass_app_for_test(mMeshFilePath);

    const auto tProcessManager = process_manager::library::make_process_managers(
        tValidatedInput.get<components::ComponentType::kProcessManager>());
    ASSERT_EQ(tProcessManager.size(), 2u);
    const auto tROLOptimize = tProcessManager[0];
    const auto tProcessManagerData = process_manager::library::make_process_manager_data(tValidatedInput);
    tROLOptimize(tProcessManagerData);

    const auto tConstraintCheckInput = input_validation::get_input_block<input_parser::constraint_check>(
        tValidatedInput.get<components::ComponentType::kProcessManager>().rawInput().at(1U));
    EXPECT_TRUE(std::filesystem::remove(tConstraintCheckInput.linearity_check_output_file_name.value().mToken));
    EXPECT_TRUE(std::filesystem::remove(tConstraintCheckInput.jacobian_check_output_file_name.value().mToken));
    EXPECT_TRUE(
        std::filesystem::remove(tConstraintCheckInput.jacobian_adjoint_consistency_output_file_name.value().mToken));
}

}  // namespace plato::integration_tests::unittest
