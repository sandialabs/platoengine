#include "plato/integration_tests/utilities/InputGeneration.hpp"

namespace plato::integration_tests::utilities
{
auto create_valid_example_input() -> input_parser::ParsedInput
{
//    return create_valid_example_objective() | create_valid_example_constraint() |
//           create_valid_density_topology_geometry() | create_valid_example_rol_optimization() |
//           create_valid_identity_filter();
    return input_parser::ParsedInput{};
}

}  // namespace plato::integration_tests::utilities
