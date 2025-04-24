#include <gtest/gtest.h>

#include "plato/criteria/library/test_utilities/ExampleInputBlocks.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/process_manager/extension/GradientCheck.hpp"
#include "plato/process_manager/library/ProcessManagerValidation.hpp"

namespace plato::process_manager::library::unittest
{
TEST(ProcessManagerValidation, InValidParsedInputNoProcessManagers)
{
    const auto tInput = input_parser::ParsedInput{};
    EXPECT_TRUE(detail::validate_at_least_one_process_manager(tInput).has_value());
}
}  // namespace plato::process_manager::library::unittest
