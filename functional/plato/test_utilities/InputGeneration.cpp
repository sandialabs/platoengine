#include "plato/test_utilities/InputGeneration.hpp"

#include <fstream>

#include "plato/input_parser/InputBlockUtilities.hpp"

namespace plato::test_utilities
{

input_parser::ParsedInput create_valid_example_input()
{
    return create_valid_example_objective() | create_valid_example_constraint() |
           create_valid_density_topology_geometry() | create_valid_example_rol_optimization();
}

input_parser::brick_shape_geometry create_valid_brick_shape_geometry()
{
    return input_parser::brick_shape_geometry{/*.mesh_name=*/input_parser::FileName{"my_mesh.exo"}};
}

std::string create_valid_brick_shape_geometry_string()
{
    return R"(
        begin brick_shape_geometry
          mesh_name my_mesh.exo
        end
        )";
}

input_parser::density_topology create_valid_density_topology_geometry()
{
    return input_parser::density_topology{/*.mesh_name = */ input_parser::FileName{"test.exo"},
                                          /*.output_name = */ input_parser::FileName{"test_out.exo"},
                                          /*.filter_type = */ input_parser::FilterTypes::kIdentity,
                                          /*.filter_radius=*/boost::none,
                                          /*.boundary_sticking_penalty=*/boost::none};
}

std::string create_valid_density_topology_geometry_string()
{
    return R"(
        begin density_topology
          mesh_name my_mesh.exo
          output_name test_out.exo
          filter_type identity
        end
        )";
}

input_parser::constraint create_valid_example_constraint()
{
    return input_parser::constraint{/*.name=*/std::string{"bike-shed"},
                                    /*.active=*/true,
                                    /*.app=*/input_parser::CodeOptions::kNodalSum,
                                    /*.shared_library_path=*/input_parser::FileName{},
                                    /*.number_of_processors=*/1u,
                                    /*.input_files=*/input_parser::FileList{{"brown.txt", "butter.txt", "sauce.txt"}},
                                    /*.equal_to=*/0.0,
                                    /*.is_linear=*/true};
}

std::string create_valid_example_constraint_string()
{
    return R"(
          begin constraint test
            active true
            app nodal_sum
            number_of_processors 1
            input_files test-input.inp
            equal_to 13
            is_linear true
          end
       )";
}

input_parser::objective create_valid_example_objective()
{
    return input_parser::objective{/*.name=*/std::string{"bike-shed"},
                                   /*.active=*/true,
                                   /*.app=*/input_parser::CodeOptions::kNodalSum,
                                   /*.shared_library_path=*/input_parser::FileName{},
                                   /*.number_of_processors=*/1u,
                                   /*.input_files=*/input_parser::FileList{{"brown.txt", "butter.txt", "sauce.txt"}},
                                   /*.aggregation_weight=*/13.0,
                                   /*.objective_type=*/input_parser::ObjectiveTypes::kMaximize};
}

std::string create_valid_example_objective_string()
{
    return R"(
          begin objective test
            active true
            app nodal_sum
            number_of_processors 1
            input_files test-input.inp
            aggregation_weight 42.0
            objective_type minimize
          end
       )";
}

input_parser::rol_optimization create_valid_example_rol_optimization()
{
    return input_parser::rol_optimization{/*.input_file_name=*/boost::none,
                                          /*.max_iterations =  */ 42,
                                          /*.step_tolerance = */ 1e-7,
                                          /*.gradient_tolerance = */ 1e-9};
}

std::string create_valid_example_rol_optimization_string()
{
    return R"(
          begin rol_optimization
            max_iterations 666
            step_tolerance 1e-4
            gradient_tolerance 1e-6
          end
       )";
}

input_parser::gradient_check create_valid_example_gradient_check()
{
    return input_parser::gradient_check{/*.output_file_name=*/input_parser::FileName{"gradient_check.txt"},
                                        /*.number_of_steps=*/12,
                                        /*.initial_direction_magnitude=*/0.5,
                                        /*.step_size_reduction_factor = */ 0.5,
                                        /*.random_direction_seed = */ 42};
}

input_parser::constraint_check create_valid_example_constraint_check()
{
    return input_parser::constraint_check{
        /*.linearity_check_output_file_name=*/input_parser::FileName{"constraint_linearity_check.txt"},
        /*.jacobian_check_output_file_name=*/input_parser::FileName{"constraint_jacobian_check.txt"},
        /*.jacobian_adjoint_consistency_output_file_name=*/
        input_parser::FileName{"constraint_jacobian_adjoint_consistency_check.txt"},
        /*.number_of_steps*/ 10u,
        /*.initial_direction_magnitude*/ 1.0,
        /*.step_size_reduction_factor*/ 0.1,
        /*.random_direction_seed*/ 123};
}

input_parser::sensitivity_check create_valid_example_sensitivity_check()
{
    return input_parser::sensitivity_check{/*.output_file_name=*/input_parser::FileName{"sensitivity_check.txt"}};
}
}  // namespace plato::test_utilities