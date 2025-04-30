#ifndef PLATO_TEST_UTILITIES_INPUTGENERATION
#define PLATO_TEST_UTILITIES_INPUTGENERATION

#include <string>
#include <string_view>

namespace plato::test_utilities
{
[[nodiscard]] std::string create_valid_brick_shape_geometry_string();
[[nodiscard]] std::string create_valid_density_topology_geometry_string();
[[nodiscard]] std::string create_valid_example_objective_string();
[[nodiscard]] std::string create_valid_example_constraint_string();
[[nodiscard]] std::string create_valid_example_rol_optimization_string();
[[nodiscard]] std::string create_valid_identity_filter_string();
}  // namespace plato::test_utilities
#endif
