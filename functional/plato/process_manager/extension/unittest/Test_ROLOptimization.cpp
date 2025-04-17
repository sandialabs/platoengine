#include <gtest/gtest.h>

#include <filesystem>

#include "plato/criteria/library/ObjectiveInputBlock.hpp"
#include "plato/geometry/extension/BrickShapeGeometry.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/process_manager/extension/ROLOptimization.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/process_manager/library/ProcessManagerRegistration.hpp"
#include "plato/test_utilities/InputGeneration.hpp"

namespace plato::process_manager::extension::unittest
{
namespace
{
constexpr std::string_view kROLOptimizerFileName = "ROL_Optimizer.txt";
}

TEST(ROLOptimization, Create)
{
    const auto tInputDeck = geometry::extension::create_valid_brick_shape_geometry_input() |
                            criteria::library::create_valid_example_objective_input() |
                            create_valid_example_rol_optimization_input();
    const auto tValidatedInput = input_validation::make_validated_input(tInputDeck);
    ASSERT_TRUE(tValidatedInput.hasValue());
    const auto tProblem = library::make_process_manager_data(tValidatedInput.value());
    const auto tAllProcessManagerInputs = tValidatedInput.value().get<input_parser::ComponentType::kProcessManager>();
    ASSERT_EQ(tAllProcessManagerInputs.rawInput().size(), 1u);
    const auto tROLOptimization = ROLOptimization{tAllProcessManagerInputs.rawInput().front()};
    tROLOptimization.run(tProblem);
    EXPECT_TRUE(std::filesystem::exists(kROLOptimizerFileName));
    EXPECT_TRUE(std::filesystem::remove(kROLOptimizerFileName));
}

TEST(ROLOptimization, Registration)
{
    EXPECT_TRUE(library::is_new_process_manager_function_registered("new_rol_optimization"));
}

}  // namespace plato::process_manager::extension::unittest
