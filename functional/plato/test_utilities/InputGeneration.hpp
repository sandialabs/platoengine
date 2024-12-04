#ifndef PLATO_TEST_UTILITIES_INPUTGENERATION
#define PLATO_TEST_UTILITIES_INPUTGENERATION

#include <string>
#include <string_view>

#include "plato/input_parser/InputBlocks.hpp"

namespace plato::test_utilities
{
[[nodiscard]] input_parser::ParsedInput create_valid_example_input();

[[nodiscard]] input_parser::brick_shape_geometry create_valid_brick_shape_geometry();
[[nodiscard]] std::string create_valid_brick_shape_geometry_string();

[[nodiscard]] input_parser::density_topology create_valid_density_topology_geometry();
[[nodiscard]] auto create_valid_density_topology_geometry_with_element_centered_kernel_filter()
    -> std::pair<input_parser::density_topology, input_parser::kernel_filter>;
[[nodiscard]] std::string create_valid_density_topology_geometry_string();

[[nodiscard]] input_parser::level_set_topology create_valid_level_set_topology_geometry();
[[nodiscard]] std::string create_valid_level_set_topology_geometry_string();

[[nodiscard]] input_parser::objective create_valid_example_objective();
[[nodiscard]] std::string create_valid_example_objective_string();

[[nodiscard]] input_parser::constraint create_valid_example_constraint();
[[nodiscard]] std::string create_valid_example_constraint_string();

[[nodiscard]] input_parser::rol_optimization create_valid_example_rol_optimization();
[[nodiscard]] std::string create_valid_example_rol_optimization_string();

[[nodiscard]] input_parser::gradient_check create_valid_example_gradient_check();

[[nodiscard]] input_parser::constraint_check create_valid_example_constraint_check();

[[nodiscard]] input_parser::sensitivity_check create_valid_example_sensitivity_check();

[[nodiscard]] input_parser::identity_filter create_valid_identity_filter();

[[nodiscard]] std::string create_valid_identity_filter_string();

[[nodiscard]] input_parser::helmholtz_filter create_valid_helmholtz_filter();

[[nodiscard]] input_parser::kernel_filter create_valid_kernel_filter();

[[nodiscard]] input_parser::snopt_optimization create_valid_example_snopt_optimization();

[[nodiscard]] input_parser::kernel_filter create_valid_element_centered_kernel_filter();

}  // namespace plato::test_utilities
#endif
