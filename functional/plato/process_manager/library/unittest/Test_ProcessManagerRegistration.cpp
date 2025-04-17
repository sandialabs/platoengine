#include <gtest/gtest.h>

#include <variant>

#include "plato/input_parser/InputBlocks.hpp"
#include "plato/process_manager/library/ProcessManagerRegistration.hpp"
#include "plato/process_manager/library/StageOrdering.hpp"

namespace plato::process_manager::library::unittest
{
namespace
{
[[nodiscard]] auto make_test_process_manager_function(const library::NewValidatedProcessManagerInput&)
    -> StageAndProcessManager
{
    return {RunStage::kValidate, [](const ProcessManagerData&) {}};
}

[[maybe_unused]] static auto kTestProcessManagerRegistration =
    NewProcessManagerRegistration{"test", [](const NewValidatedProcessManagerInput& aValidInput)
                                  { return make_test_process_manager_function(aValidInput); }};
}  // namespace

TEST(ProcessManagerRegistration, TestProcessManager)
{
    EXPECT_TRUE(is_new_process_manager_function_registered("test"));
}

TEST(ProcessManagerRegistration, NonexistantProcessManager)
{
    EXPECT_FALSE(is_new_process_manager_function_registered("flem"));
}
}  // namespace plato::process_manager::library::unittest
