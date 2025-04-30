#ifndef PLATO_CRITERIA_EXTENSION_TEST_UTILITIES
#define PLATO_CRITERIA_EXTENSION_TEST_UTILITIES

#include "plato/criteria/library/ConstraintInputBlock.hpp"
#include "plato/criteria/library/ObjectiveInputBlock.hpp"

namespace plato::criteria::library::test_utilities
{
/// @brief Creates a valid example Constraint struct, useful for testing.
[[nodiscard]] auto create_valid_example_constraint_input() -> input_parser::constraint;

/// @brief Creates a valid example Objective struct, useful for testing.
[[nodiscard]] auto create_valid_example_objective_input() -> input_parser::objective;
}  // namespace plato::criteria::library::test_utilities

#endif
