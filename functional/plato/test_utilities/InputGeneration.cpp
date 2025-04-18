#include "plato/test_utilities/InputGeneration.hpp"

#include <fstream>
#include <optional>

#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_parser/InputDefinitions.hpp"

namespace plato::test_utilities
{
std::string create_valid_brick_shape_geometry_string()
{
    return R"(
        begin new_brick_shape_geometry
          mesh_name my_mesh.exo
        end
        )";
}

std::string create_valid_density_topology_geometry_string()
{
    return R"(
        begin new_density_topology
          mesh_name test.exo
          output_name test_out.exo
          initial_density_value 0.5
        end
        )";
}

std::string create_valid_example_constraint_string()
{
    return R"(
          begin new_constraint test
            active true
            app platoengine
            criterion nodal_sum
            number_of_processors 1
            input_files test-input.inp
            constraint_value 13
            is_linear true
            constraint_type equal_to
          end
       )";
}

std::string create_valid_example_objective_string()
{
    return R"(
          begin new_objective test
            active true
            app platoengine
            criterion nodal_sum
            number_of_processors 1
            input_files test-input.inp
            aggregation_weight 42.0
          end
       )";
}

std::string create_valid_example_rol_optimization_string()
{
    return R"(
          begin new_rol_optimization
            max_iterations 666
            step_tolerance 1e-4
            gradient_tolerance 1e-6
          end
       )";
}

std::string create_valid_identity_filter_string()
{
    return R"(
          begin identity_filter
          end
       )";
}

}  // namespace plato::test_utilities
