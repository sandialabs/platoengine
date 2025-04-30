#include <gtest/gtest.h>

#include "plato/integration_tests/utilities/MassAppTestUtilities.hpp"
#include "plato/process_manager/extension/ConstraintCheck.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/process_manager/library/ProcessManagerFactory.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"

namespace plato::integration_tests::unittest
{
using third_party_integration::stk_io::test_utilities::OneBlock3x1x1HexMesh;

TEST_F(OneBlock3x1x1HexMesh, MassVectorConstraintRegisterAndRun)
{
    const auto [tTempDirector, tValidatedInput] = integration_tests::utilities::setup_mass_app_for_test(mMeshFilePath);

    const auto tProcessManager = process_manager::library::make_process_managers(
        tValidatedInput.get<input_parser::ComponentType::kProcessManager>());
    ASSERT_EQ(tProcessManager.size(), 2u);
    const auto tROLOptimize = tProcessManager[0];
    const auto tProcessManagerData = process_manager::library::make_process_manager_data(tValidatedInput);
    tROLOptimize(tProcessManagerData);

    const auto tConstraintCheckInput = input_validation::get_input_block<input_parser::constraint_check>(
        tValidatedInput.get<input_parser::ComponentType::kProcessManager>().rawInput().at(1U));
    EXPECT_TRUE(std::filesystem::remove(tConstraintCheckInput.linearity_check_output_file_name.value().mToken));
    EXPECT_TRUE(std::filesystem::remove(tConstraintCheckInput.jacobian_check_output_file_name.value().mToken));
    EXPECT_TRUE(
        std::filesystem::remove(tConstraintCheckInput.jacobian_adjoint_consistency_output_file_name.value().mToken));
}

}  // namespace plato::integration_tests::unittest
