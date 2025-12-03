#include "plato/process_manager/extension/test_utilities/ExampleInputBlocks.hpp"

namespace plato::process_manager::extension::test_utilities
{
auto create_valid_example_constraint_check_input() -> input_parser::constraint_check
{
    return input_parser::constraint_check{
        /*.linearity_check_output_file_name=*/input_parser::FileName{"constraint_linearity_check.txt"},
        /*.jacobian_check_output_file_name=*/input_parser::FileName{"constraint_jacobian_check.txt"},
        /*.jacobian_adjoint_consistency_output_file_name=*/
        input_parser::FileName{"constraint_jacobian_adjoint_consistency_check.txt"},
        /*.number_of_steps*/ 10u,
        /*.initial_direction_magnitude*/ 1.0,
        /*.step_size_reduction_factor*/ 0.1,
        /*.random_direction_seed*/ 123,
        /*.direction_vector_type = */ input_parser::DirectionVectorTypes::kRandom};
}

auto create_valid_example_gradient_check_input() -> input_parser::gradient_check
{
    return input_parser::gradient_check{/*.output_file_name=*/input_parser::FileName{"gradient_check.txt"},
                                        /*.number_of_steps=*/12,
                                        /*.initial_direction_magnitude=*/0.5,
                                        /*.step_size_reduction_factor = */ 0.5,
                                        /*.random_direction_seed = */ 42,
                                        /*.direction_vector_type = */ input_parser::DirectionVectorTypes::kRandom};
}

auto create_valid_example_rol_optimization_input() -> input_parser::rol_optimization
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

auto create_valid_example_sensitivity_check_input() -> input_parser::sensitivity_check
{
    return input_parser::sensitivity_check{/*.output_file_name=*/input_parser::FileName{"sensitivity_check.txt"}};
}

}  // namespace plato::process_manager::extension::test_utilities
