#include <gtest/gtest.h>

#include "plato/criteria/library/test_utilities/ExampleInputBlocks.hpp"
#include "plato/geometry/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/process_manager/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/process_manager/library/ProcessManagerFactory.hpp"

namespace plato::integration_tests::serial
{
namespace
{
void verify_number_of_process_managers(const input_parser::ParsedInput& aParsedInput,
                                       unsigned int aNumberOfProcessManangers)
{
    const auto tInput = input_validation::make_validated_input(aParsedInput).value();
    const auto tProcessManagers = tInput.get<components::ComponentType::kProcessManager>().rawInput();
    EXPECT_EQ(tProcessManagers.size(), aNumberOfProcessManangers);
}

}  // namespace

TEST(ProcessManagerFactory, RightNumberOfProcessManagers)
{
    auto tRawInput = geometry::extension::test_utilities::create_valid_brick_shape_geometry_input() |
                     criteria::library::test_utilities::create_valid_example_objective_input() |
                     process_manager::extension::test_utilities::create_valid_example_gradient_check_input() |
                     process_manager::extension::test_utilities::create_valid_example_gradient_check_input();
    verify_number_of_process_managers(tRawInput, 2U);
    tRawInput = tRawInput | process_manager::extension::test_utilities::create_valid_example_sensitivity_check_input();
    verify_number_of_process_managers(tRawInput, 3U);
}

}  // namespace plato::integration_tests::serial
