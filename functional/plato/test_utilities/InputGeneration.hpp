#ifndef PLATO_TEST_UTILITIES_INPUTGENERATION
#define PLATO_TEST_UTILITIES_INPUTGENERATION

#include <string>
#include <string_view>

namespace plato::test_utilities
{
[[nodiscard]] auto create_valid_brick_shape_geometry_string() -> std::string;
[[nodiscard]] auto create_valid_density_topology_geometry_string() -> std::string;
[[nodiscard]] auto create_valid_example_objective_string() -> std::string;
[[nodiscard]] auto create_valid_example_constraint_string() -> std::string;
[[nodiscard]] auto create_valid_example_rol_optimization_string() -> std::string;
[[nodiscard]] auto create_valid_identity_filter_string() -> std::string;
}  // namespace plato::test_utilities
#endif
