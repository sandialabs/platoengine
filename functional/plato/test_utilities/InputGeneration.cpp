#include "plato/test_utilities/InputGeneration.hpp"

#include <fstream>
#include <optional>

#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_parser/InputDefinitions.hpp"

namespace plato::test_utilities
{

input_parser::ParsedInput create_valid_example_input()
{
    return create_valid_example_objective() | create_valid_example_constraint() |
           create_valid_density_topology_geometry() | create_valid_example_rol_optimization() |
           create_valid_identity_filter();
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
    return input_parser::density_topology{/*.mesh_name=*/input_parser::FileName{"test.exo"},
                                          /*.output_name=*/input_parser::FileName{"test_out.exo"},
                                          /*.fixed_blocks=*/{},
                                          /*.filter=*/boost::none,
                                          /*.initial_density_value=*/0.5,
                                          /*.initial_field_name=*/boost::none};
}

auto create_valid_density_topology_geometry_with_element_centered_kernel_filter()
    -> std::pair<input_parser::density_topology, input_parser::kernel_filter>
{
    const auto tDensity = create_valid_density_topology_geometry();
    const auto tFilter = create_valid_element_centered_kernel_filter();
    return {tDensity, tFilter};
}

std::string create_valid_density_topology_geometry_string()
{
    return R"(
        begin density_topology
          mesh_name test.exo
          output_name test_out.exo
          initial_density_value 0.5
        end
        )";
}

input_parser::level_set_topology create_valid_level_set_topology_geometry()
{
    return input_parser::level_set_topology{/*.mesh_name = */ input_parser::FileName{"bg.exo"},
                                            /*.output_name = */ input_parser::FileName{"out.exo"},
                                            /*.include_void_region = */ false,
                                            /*.sphere_pattern_bbox_min_x = */ 0.0,
                                            /*.sphere_pattern_bbox_min_y = */ 0.0,
                                            /*.sphere_pattern_bbox_min_z = */ 0.0,
                                            /*.sphere_pattern_bbox_max_x = */ 1.0,
                                            /*.sphere_pattern_bbox_max_y = */ 1.0,
                                            /*.sphere_pattern_bbox_max_z = */ 1.0,
                                            /*.sphere_pattern_radius = */ 0.25,
                                            /*.sphere_pattern_spacing = */ 100.0,
                                            /*.level_set_lower_bound = */ -1.0,
                                            /*.level_set_upper_bound = */ 1.0,
                                            /*.filter=*/boost::none,
                                            /*.fixed_blocks=*/boost::none,
                                            /*.initial_field_name=*/boost::none};
}

input_parser::level_set_topology create_valid_level_set_topology_geometry_initialize_from_field()
{
    return input_parser::level_set_topology{/*.mesh_name = */
                                            input_parser::FileName{"mesh.exo"},
                                            /*.output_name = */ input_parser::FileName{"level-set-output.exo"},
                                            /*.include_void_region = */ true,
                                            /*.sphere_pattern_bbox_min_x = */ boost::none,
                                            /*.sphere_pattern_bbox_min_y = */ boost::none,
                                            /*.sphere_pattern_bbox_min_z = */ boost::none,
                                            /*.sphere_pattern_bbox_max_x = */ boost::none,
                                            /*.sphere_pattern_bbox_max_y = */ boost::none,
                                            /*.sphere_pattern_bbox_max_z = */ boost::none,
                                            /*.sphere_pattern_radius = */ boost::none,
                                            /*.sphere_pattern_spacing = */ boost::none,
                                            /*.level_set_lower_bound = */ -1.0,
                                            /*.level_set_upper_bound = */ 1.0,
                                            /*.filter=*/boost::none,
                                            /*.fixed_blocks=*/boost::none,
                                            /*.initial_field_name=*/input_parser::IdentifierString{"density"}};
}

input_parser::constraint create_valid_example_constraint()
{
    return input_parser::constraint{/*.name=*/std::string{"bike-shed"},
                                    /*.active=*/true,
                                    /*.app=*/input_parser::AppName{std::string{input_parser::kBuiltinAppName}},
                                    /*.criterion=*/input_parser::CriterionName{"nodal_sum"},
                                    /*.number_of_processors=*/1u,
                                    /*.input_files=*/input_parser::FileList{{"brown.txt", "butter.txt", "sauce.txt"}},
                                    /*.constraint_value=*/0.0,
                                    /*.is_linear=*/true,
                                    /*.constraint_type=*/input_parser::ConstraintTypes::kEqualTo};
}

std::string create_valid_example_constraint_string()
{
    return R"(
          begin constraint test
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

input_parser::objective create_valid_example_objective()
{
    return input_parser::objective{/*.name=*/std::string{"bike-shed"},
                                   /*.active=*/true,
                                   /*.app=*/input_parser::AppName{std::string{input_parser::kBuiltinAppName}},
                                   /*.criterion=*/input_parser::CriterionName{"nodal_sum"},
                                   /*.number_of_processors=*/1u,
                                   /*.input_files=*/input_parser::FileList{{"brown.txt", "butter.txt", "sauce.txt"}},
                                   /*.aggregation_weight=*/13.0};
}

std::string create_valid_example_objective_string()
{
    return R"(
          begin objective test
            active true
            app platoengine
            criterion nodal_sum
            number_of_processors 1
            input_files test-input.inp
            aggregation_weight 42.0
          end
       )";
}

input_parser::rol_optimization create_valid_example_rol_optimization()
{
    return input_parser::rol_optimization{/*.input_file_name=*/boost::none,
                                          /*.export_settings_file_name=*/input_parser::FileName{"output_rol.xml"},
                                          /*.max_iterations =  */ 42,
                                          /*.step_tolerance = */ 1e-7,
                                          /*.gradient_tolerance = */ 1e-5,
                                          /*.initial_search_radius = */ 15,
                                          /*.verbose_output = */ false,
                                          /*.approximate_hessian = */ false,
                                          /*.output_design_history = */ false};
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

input_parser::identity_filter create_valid_identity_filter()
{
    return input_parser::identity_filter{/*.filter_radius = */ boost::none};
}

std::string create_valid_identity_filter_string()
{
    return R"(
          begin identity_filter
          end
       )";
}

input_parser::helmholtz_filter create_valid_helmholtz_filter()
{
    return input_parser::helmholtz_filter{/*.filter_radius=*/91.0,
                                          /*.use_relative_radius=*/boost::none,
                                          /*.boundary_sticking_penalty=*/1.0};
}

input_parser::kernel_filter create_valid_kernel_filter()
{
    return input_parser::kernel_filter{/*.filter_radius=*/17.0,
                                       /*.centering_type=*/input_parser::KernelFilterCenteringTypes::kNodeCentered,
                                       /*.use_relative_radius=*/boost::none,
                                       /*.number_of_processors*/ 1};
}

input_parser::snopt_optimization create_valid_example_snopt_optimization()
{
    return input_parser::snopt_optimization{/*.input_file_name=*/boost::none,
                                            /*.max_iterations=*/10,
                                            /*.time_limit_in_minutes=*/0,
                                            /*.output_design_history=*/false};
}

input_parser::kernel_filter create_valid_element_centered_kernel_filter()
{
    return input_parser::kernel_filter{/*.filter_radius=*/2.0,
                                       /*.centering_type=*/input_parser::KernelFilterCenteringTypes::kElementCentered,
                                       /*.use_relative_radius=*/false,
                                       /*.number_of_processors*/ 1};
}
}  // namespace plato::test_utilities
