#ifndef PLATO_INTEGRATION_TESTS_UTILITIES_INPUTGENERATION
#define PLATO_INTEGRATION_TESTS_UTILITIES_INPUTGENERATION

#include <filesystem>

#include "plato/input_parser/ParsedInput.hpp"

namespace plato::integration_tests::utilities
{
/// @brief Generates a valid complete input deck that may be used for testing.
[[nodiscard]] auto create_valid_example_input() -> input_parser::NewParsedInput;

/// @brief Generates valid density topology and kernel filter inputs for testing.
[[nodiscard]] auto create_valid_density_topology_geometry_with_element_centered_kernel_filter_input(
    const std::filesystem::path& aMeshPath) -> input_parser::NewParsedInput;

}  // namespace plato::integration_tests::utilities

#endif
