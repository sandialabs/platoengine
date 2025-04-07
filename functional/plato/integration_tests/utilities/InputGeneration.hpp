#ifndef PLATO_INTEGRATION_TESTS_UTILITIES_INPUTGENERATION
#define PLATO_INTEGRATION_TESTS_UTILITIES_INPUTGENERATION

#include <string>
#include <string_view>

#include "plato/input_parser/InputBlocks.hpp"
#include "plato/input_parser/ParsedInput.hpp"

namespace plato::integration_tests::utilities
{
/// @brief Generates a valid complete input deck that may be used for testing.
[[nodiscard]] auto create_valid_example_input() -> input_parser::ParsedInput;
}  // namespace plato::integration_tests::utilities

#endif
