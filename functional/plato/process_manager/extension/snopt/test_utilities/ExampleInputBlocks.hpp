#ifndef PLATO_PROCESSMANAGER_EXTENSION_SNOPT_TESTUTILITIES_EXAMPLEINPUTBLOCKS
#define PLATO_PROCESSMANAGER_EXTENSION_SNOPT_TESTUTILITIES_EXAMPLEINPUTBLOCKS

#include "plato/process_manager/extension/snopt/SNOPTOptimization.hpp"

namespace plato::process_manager::extension::snopt::test_utilities
{

/// @brief Creates a valid example SNOPTOptimization input struct, useful for tests.
[[nodiscard]] auto create_valid_example_snopt_optimization_input() -> input_parser::snopt_optimization;

}  // namespace plato::process_manager::extension::snopt::test_utilities

#endif
