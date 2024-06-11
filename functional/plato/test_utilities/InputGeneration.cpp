#include "plato/test_utilities/InputGeneration.hpp"

#include <fstream>

namespace plato::test_utilities
{
void create_input_file(const std::filesystem::path& aTestFileName)
{
    std::ofstream tOutFile(aTestFileName);
    const std::string tInput =
        R"(
          begin brick_shape_geometry
            mesh_name my_mesh.exo
          end
          begin objective test
            active true
            app nodal_sum
            number_of_processors 4
            input_files test-input.inp
            aggregation_weight 42.0
            objective_type minimize
          end
          begin rol_optimization
            input_file_name its-a_file.txt
            step_tolerance 10
            gradient_tolerance 100.0
            
          end
       )";
    tOutFile << tInput << std::endl;
    tOutFile.close();
}

[[nodiscard]] std::string create_valid_example_input_string()
{
    const std::string tConstraintInput = create_valid_example_constraint_string();
    const std::string tObjectiveInput = create_valid_example_objective_string();
    const std::string tGeometryInput = create_valid_density_topology_geometry_string();
    const std::string tOptimizerInput = create_valid_example_rol_optimization_string();

    return tConstraintInput + tObjectiveInput + tGeometryInput + tOptimizerInput;
}

input_parser::ParsedInput create_valid_example_input()
{
    return input_parser::ParsedInput{/*.mObjectives=*/{create_valid_example_objective()},
                                     /*.mConstraints=*/{create_valid_example_constraint()},
                                     /*.mBrickShapeGeometry=*/boost::none,
                                     /*.mDensityTopology = */ create_valid_density_topology_geometry(),
                                     /*.mLevelsetTopology = */ create_valid_levelset_topology_geometry(),
                                     /*.mROLOptimization = */ create_valid_example_rol_optimization(),
                                     /*.mGradientCheck=*/boost::none,
                                     /*.mSensitivityCheck=*/boost::none};
}

input_parser::ParsedInput create_valid_example_input_with_gradient_check()
{
    auto tInput = create_valid_example_input();
    tInput.mGradientCheck = create_valid_example_gradient_check();
    return tInput;
}

input_parser::ParsedInput create_valid_shape_geometry_example_input_with_gradient_check()
{
    input_parser::ParsedInput tInputDeck = create_valid_example_input_with_gradient_check();
    tInputDeck.mDensityTopology = boost::none;
    tInputDeck.mBrickShapeGeometry = create_valid_brick_shape_geometry();
    return tInputDeck;
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

input_parser::levelset_topology create_valid_levelset_topology_geometry()
{
    return input_parser::levelset_topology{/*.background_mesh_name = */ input_parser::FileName{"bg.exo"},
                                           /*.cut_mesh_name = */ input_parser::FileName{"cut.exo"},
                                           /*.output_mesh_name = */ input_parser::FileName{"out.exo"},
                                           /*.sphere_pattern_bbox_min_x = */ 0.0,
                                           /*.sphere_pattern_bbox_min_y = */ 0.0,
                                           /*.sphere_pattern_bbox_min_z = */ 0.0,
                                           /*.sphere_pattern_bbox_max_x = */ 1.0,
                                           /*.sphere_pattern_bbox_max_y = */ 1.0,
                                           /*.sphere_pattern_bbox_max_z = */ 1.0,
                                           /*.sphere_pattern_radius = */ 0.25,
                                           /*.sphere_pattern_num_x = */ 1,
                                           /*.sphere_pattern_num_y = */ 1,
                                           /*.sphere_pattern_num_z = */ 1,
                                           /*.sphere_pattern_overlap_bbox = */ false};
}

std::string create_valid_levelset_topology_geometry_string()
{
    return R"(
        begin levelset_topology
          background_mesh_name bg.exo
          cut_mesh_name cut.exo
          output_mesh_name out.exo
        end
        )";
}

input_parser::constraint create_valid_example_constraint()
{
    return input_parser::constraint{/*.name=*/std::string{"bike-shed"},
                                    /*.active=*/true,
                                    /*.app=*/input_parser::CodeOptions::kNodalSum,
                                    /*.shared_library_path=*/input_parser::FileName{},
                                    /*.number_of_processors=*/42u,
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
            number_of_processors 4
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
                                   /*.number_of_processors=*/42u,
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
            number_of_processors 4
            input_files test-input.inp
            aggregation_weight 42.0
            objective_type minimize
          end
       )";
}

std::string create_valid_example_custom_app_objective_string()
{
    return R"(
          begin objective test
            active true
            app custom_app
            shared_library_path /path/to/nothing.so
            number_of_processors 13
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

std::string create_valid_example_gradient_check_string()
{
    return R"(
          begin gradient_check
            output_file_name gradient_check_file.txt
            number_of_steps 12
            step_size_reduction_factor 0.5
            random_direction_seed 123
            initial_direction_magnitude 0.5
          end
       )";
}

input_parser::sensitivity_check create_valid_example_sensitivity_check()
{
    return input_parser::sensitivity_check{/*.output_file_name=*/input_parser::FileName{"sensitivity_check.txt"}};
}

std::string create_valid_example_sensitivity_check_string()
{
    return R"(
          begin sensitivity_check
            output_file_name sensitivity_check_file.txt
          end
       )";
}

}  // namespace plato::test_utilities