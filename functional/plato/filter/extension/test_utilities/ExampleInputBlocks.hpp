#ifndef PLATO_FILTER_EXTENSION_TEST_UTILITIES_EXAMPLEINPUTBLOCKS
#define PLATO_FILTER_EXTENSION_TEST_UTILITIES_EXAMPLEINPUTBLOCKS

#include "plato/filter/extension/HelmholtzFilter.hpp"
#include "plato/filter/extension/IdentityFilter.hpp"
#include "plato/filter/extension/KernelFilter.hpp"

namespace plato::filter::extension::test_utilities
{
/// @brief Generates a input struct for IdentityFilter, useful for tests.
[[nodiscard]] auto create_valid_identity_filter_input() -> input_parser::identity_filter;

/// @brief Creates a valid example input struct for a HelmholtzFilter, useful for testing.
[[nodiscard]] auto create_valid_helmholtz_filter_input() -> input_parser::helmholtz_filter;

/// @brief Creates a valid example KernelFilter input struct, useful for testing.
[[nodiscard]] auto create_valid_kernel_filter_input() -> input_parser::kernel_filter;

}  // namespace plato::filter::extension::test_utilities

#endif
