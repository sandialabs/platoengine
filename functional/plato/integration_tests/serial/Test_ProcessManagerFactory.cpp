#include <gtest/gtest.h>

#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/process_manager/library/ProcessManagerFactory.hpp"
#include "plato/process_manager/library/ValidatedInput.hpp"
#include "plato/test_utilities/InputGeneration.hpp"

namespace plato::integration_tests::serial
{
namespace
{
void verify_number_of_process_managers(const input_parser::ParsedInput& aParsedInput,
                                       unsigned int aNumberOfProcessManangers)
{
    const process_manager::library::ValidatedInput tInput =
        process_manager::library::make_validated_input(aParsedInput);
    const auto tProcessManagers = tInput.processManagers().rawInput();
    EXPECT_EQ(tProcessManagers.size(), aNumberOfProcessManangers);
}

}  // namespace

TEST(ProcessManagerFactory, RightNumberOfProcessManagers)
{
    input_parser::ParsedInput tRawInput =
        test_utilities::create_valid_brick_shape_geometry() | test_utilities::create_valid_example_objective() |
        test_utilities::create_valid_example_rol_optimization() | test_utilities::create_valid_example_gradient_check();
    verify_number_of_process_managers(tRawInput, 2u);
    tRawInput.mSensitivityCheck = test_utilities::create_valid_example_sensitivity_check();
    verify_number_of_process_managers(tRawInput, 3u);
}

}  // namespace plato::integration_tests::serial
