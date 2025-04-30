#ifndef PLATO_PROCESS_MANAGER_EXTENSION_TEST_UTILITIES_EXAMPLEINPUTBLOCKS
#define PLATO_PROCESS_MANAGER_EXTENSION_TEST_UTILITIES_EXAMPLEINPUTBLOCKS

#include "plato/process_manager/extension/ConstraintCheck.hpp"
#include "plato/process_manager/extension/GradientCheck.hpp"
#include "plato/process_manager/extension/ROLOptimization.hpp"
#include "plato/process_manager/extension/SensitivityCheck.hpp"

namespace plato::process_manager::extension::test_utilities
{
/// @brief Creates a valid example ConstraintCheck input struct, useful for testing.
[[nodiscard]] auto create_valid_example_constraint_check_input() -> input_parser::constraint_check;

/// @brief Creates a valid example GradientCheck input struct, useful for testing.
[[nodiscard]] auto create_valid_example_gradient_check_input() -> input_parser::gradient_check;

/// @brief Creates a valid example ROLOptimization input struct, useful for testing.
[[nodiscard]] auto create_valid_example_rol_optimization_input() -> input_parser::rol_optimization;

/// @brief Creates an valid example struct useful for testing.
[[nodiscard]] auto create_valid_example_sensitivity_check_input() -> input_parser::sensitivity_check;

}  // namespace plato::process_manager::extension::test_utilities

#endif
