#include "plato/test_utilities/InputGeneration.hpp"

#include <fstream>

namespace plato::test_utilities
{
void create_input_file(const std::filesystem::path aTestFileName)
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
          begin optimization_parameters
            input_file_name its-a_file.txt
            step_tolerance 10
            gradient_tolerance 100.0
            
          end
       )";
    tOutFile << tInput << std::endl;
    tOutFile.close();
}

input_parser::ParsedInput create_valid_example_input()
{
    return input_parser::ParsedInput{/*.mObjectives=*/{create_valid_example_objective()},
                                     /*.mConstraints=*/{create_valid_example_constraint()},
                                     /*.mBrickShapeGeometry=*/boost::none,
                                     /*.mDensityTopology = */ create_valid_density_topology_geometry(),
                                     /*.mOptimizationParameters = */ create_valid_example_optimization_parameters()};
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

input_parser::optimization_parameters create_valid_example_optimization_parameters()
{
    return input_parser::optimization_parameters{/*.input_file_name=*/boost::none,
                                                 /*.max_iterations =  */ 42,
                                                 /*.step_tolerance = */ 1e-7,
                                                 /*.gradient_tolerance = */ 1e-9};
}

std::string create_valid_example_optimization_parameters_string()
{
    return R"(
          begin optimization_parameters
            max_iterations 666
            step_tolerance 1e-4
            gradient_tolerance 1e-6
          end
       )";
}

}  // namespace plato::test_utilities