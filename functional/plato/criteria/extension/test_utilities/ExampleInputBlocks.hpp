#ifndef PLATO_CRITERIA_EXTENSION_TESTUTILITIES_EXAMPLEINPUTBLOCKS
#define PLATO_CRITERIA_EXTENSION_TESTUTILITIES_EXAMPLEINPUTBLOCKS

#include "plato/criteria/library/ObjectiveInputBlock.hpp"

namespace plato::criteria::extension::test_utilities
{
/// @brief Creates a valid example Nodal Sum objective input struct, useful for testing.
[[nodiscard]] auto create_valid_example_nodal_sum_objective_input() -> input_parser::objective;

}  // namespace plato::criteria::extension::test_utilities
#endif
