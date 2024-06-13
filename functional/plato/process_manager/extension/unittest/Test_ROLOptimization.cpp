#include <gtest/gtest.h>

#include <filesystem>

#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/process_manager/extension/ROLOptimization.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/process_manager/library/ProcessManagerRegistration.hpp"
#include "plato/process_manager/library/ValidatedInput.hpp"
#include "plato/test_utilities/InputGeneration.hpp"

namespace plato::process_manager::extension::unittest
{
namespace
{
constexpr std::string_view kROLOptimizerFileName = "ROL_Optimizer.txt";
}

TEST(ROLOptimization, Create)
{
    const input_parser::ParsedInput tInputDeck = test_utilities::create_valid_brick_shape_geometry() |
                                                 test_utilities::create_valid_example_objective() |
                                                 test_utilities::create_valid_example_rol_optimization();
    const auto tValidatedInput = library::make_validated_input(tInputDeck);
    const library::ProcessManagerData tProblem = library::make_process_manager_data(tValidatedInput);
    const library::ValidatedProcessManagerInputVector tAllProcessManagerInputs = tValidatedInput.processManagers();
    ASSERT_EQ(tAllProcessManagerInputs.rawInput().size(), 1u);
    const auto tROLOptimization = ROLOptimization{
        library::process_manager_input<input_parser::rol_optimization>(tAllProcessManagerInputs.rawInput().front())};
    tROLOptimization.run(tProblem);
    EXPECT_TRUE(std::filesystem::exists(kROLOptimizerFileName));
    EXPECT_TRUE(std::filesystem::remove(kROLOptimizerFileName));
}

}  // namespace plato::process_manager::extension::unittest