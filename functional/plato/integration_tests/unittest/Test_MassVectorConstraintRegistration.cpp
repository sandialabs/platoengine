#include <gtest/gtest.h>

#include "plato/integration_tests/utilities/MassAppTestUtilities.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/process_manager/library/ProcessManagerFactory.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"

namespace plato::integration_tests::unittest
{
using third_party_integration::stk_io::test_utilities::OneBlock3x1x1HexMesh;

TEST_F(OneBlock3x1x1HexMesh, MassVectorConstraintRegisterAndRun)
{
    const auto tAppName = input_parser::AppName{"test-mass-app"};
    const auto tConfigurationTempDirectory =
        integration_tests::utilities::register_test_mass_app(tAppName.mToken, boost::mpi::communicator{});

    const auto tCriterionName = input_parser::CriterionName{"mass"};
    const auto tValidatedInput = integration_tests::utilities::create_test_mass_vector_constraint_input(
        tAppName, tCriterionName, std::string{mMeshFilePath});

    const auto tProcessManager = process_manager::library::make_process_managers(tValidatedInput.processManagers());
    ASSERT_EQ(tProcessManager.size(), 2u);
    const auto tROLOptimize = tProcessManager[0];
    const auto tProcessManagerData = process_manager::library::make_process_manager_data(tValidatedInput);
    tROLOptimize(tProcessManagerData);

    const auto tConstraintCheckInput = process_manager::library::process_manager_input<input_parser::constraint_check>(
        tValidatedInput.processManagers().rawInput().at(1U));
    EXPECT_TRUE(
        std::filesystem::remove(tConstraintCheckInput.rawInput().linearity_check_output_file_name.value().mToken));
    EXPECT_TRUE(
        std::filesystem::remove(tConstraintCheckInput.rawInput().jacobian_check_output_file_name.value().mToken));
    EXPECT_TRUE(std::filesystem::remove(
        tConstraintCheckInput.rawInput().jacobian_adjoint_consistency_output_file_name.value().mToken));
}

}  // namespace plato::integration_tests::unittest
