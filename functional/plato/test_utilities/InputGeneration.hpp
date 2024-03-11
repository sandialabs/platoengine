#ifndef PLATO_TEST_UTILITIES_INPUTGENERATION
#define PLATO_TEST_UTILITIES_INPUTGENERATION

#include <filesystem>
#include <string>
#include <string_view>

#include "plato/input_parser/InputBlocks.hpp"

namespace plato::test_utilities
{
void create_input_file(const std::filesystem::path aTestFileName);

[[nodiscard]] input_parser::ParsedInput create_valid_example_input();

[[nodiscard]] input_parser::brick_shape_geometry create_valid_brick_shape_geometry();
[[nodiscard]] std::string create_valid_brick_shape_geometry_string();

[[nodiscard]] input_parser::density_topology create_valid_density_topology_geometry();
[[nodiscard]] std::string create_valid_density_topology_geometry_string();

[[nodiscard]] input_parser::objective create_valid_example_objective();
[[nodiscard]] std::string create_valid_example_objective_string();
[[nodiscard]] std::string create_valid_example_custom_app_objective_string();

[[nodiscard]] input_parser::constraint create_valid_example_constraint();
[[nodiscard]] std::string create_valid_example_constraint_string();

[[nodiscard]] input_parser::optimization_parameters create_valid_example_optimization_parameters();
[[nodiscard]] std::string create_valid_example_optimization_parameters_string();

}  // namespace plato::test_utilities
#endif