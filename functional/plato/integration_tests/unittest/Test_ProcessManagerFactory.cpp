#include <gtest/gtest.h>

#include "plato/criteria/library/ObjectiveInputBlock.hpp"
#include "plato/geometry/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/process_manager/extension/GradientCheck.hpp"
#include "plato/process_manager/extension/ROLOptimization.hpp"
#include "plato/process_manager/extension/SensitivityCheck.hpp"
#include "plato/process_manager/library/ProcessManagerFactory.hpp"

namespace plato::integration_tests::serial
{
namespace
{
void verify_number_of_process_managers(const input_parser::ParsedInput& aParsedInput,
                                       unsigned int aNumberOfProcessManangers)
{
    const auto tInput = input_validation::make_validated_input(aParsedInput).value();
    const auto tProcessManagers = tInput.get<input_parser::ComponentType::kProcessManager>().rawInput();
    EXPECT_EQ(tProcessManagers.size(), aNumberOfProcessManangers);
}

}  // namespace

TEST(ProcessManagerFactory, RightNumberOfProcessManagers)
{
    auto tRawInput = geometry::extension::test_utilities::create_valid_brick_shape_geometry_input() |
                     criteria::library::create_valid_example_objective_input() |
                     process_manager::extension::create_valid_example_rol_optimization_input() |
                     process_manager::extension::create_valid_example_gradient_check_input();
    verify_number_of_process_managers(tRawInput, 2u);
    tRawInput = tRawInput | process_manager::extension::create_valid_example_sensitivity_check_input();
    verify_number_of_process_managers(tRawInput, 3u);
}

}  // namespace plato::integration_tests::serial
