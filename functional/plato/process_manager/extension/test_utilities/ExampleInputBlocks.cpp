#include "plato/process_manager/extension/test_utilities/ExampleInputBlocks.hpp"

#include <boost/none.hpp>

#include "plato/input_parser/FileList.hpp"
#include "plato/process_manager/extension/ConstraintCheck.hpp"
#include "plato/process_manager/extension/DirectionVectorTypes.hpp"
#include "plato/process_manager/extension/GradientCheck.hpp"
#include "plato/process_manager/extension/ROLOptimization.hpp"
#include "plato/process_manager/extension/SensitivityCheck.hpp"

namespace plato::process_manager::extension::test_utilities
{
auto create_valid_example_constraint_check_input() -> input_parser::constraint_check
{
    constexpr auto tArbitraryNumberOfSteps = 10U;
    constexpr auto tArbitraryStepSizeReductionFactor = 0.1;
    constexpr auto tArbitraryRandomSeed = 123;
    return input_parser::constraint_check{
        /*.linearity_check_output_file_name=*/input_parser::FileName{"constraint_linearity_check.txt"},
        /*.jacobian_check_output_file_name=*/input_parser::FileName{"constraint_jacobian_check.txt"},
        /*.jacobian_adjoint_consistency_output_file_name=*/
        input_parser::FileName{"constraint_jacobian_adjoint_consistency_check.txt"},
        /*.number_of_steps*/ tArbitraryNumberOfSteps,
        /*.initial_direction_magnitude*/ 1.0,
        /*.step_size_reduction_factor*/ tArbitraryStepSizeReductionFactor,
        /*.random_direction_seed*/ tArbitraryRandomSeed,
        /*.direction_vector_type = */ input_parser::DirectionVectorTypes::kRandom};
}

auto create_valid_example_gradient_check_input() -> input_parser::gradient_check
{
    constexpr auto tArbitraryNumberOfSteps = 12U;
    constexpr auto tArbitraryInitialDirectionMagnitude = 0.5;
    constexpr auto tArbitraryStepSizeReductionFactor = 0.5;
    constexpr auto tArbitraryRandomSeed = 123;
    return input_parser::gradient_check{/*.output_file_name=*/input_parser::FileName{"gradient_check.txt"},
                                        /*.number_of_steps=*/tArbitraryNumberOfSteps,
                                        /*.initial_direction_magnitude=*/tArbitraryInitialDirectionMagnitude,
                                        /*.step_size_reduction_factor = */ tArbitraryStepSizeReductionFactor,
                                        /*.random_direction_seed = */ tArbitraryRandomSeed,
                                        /*.direction_vector_type = */ input_parser::DirectionVectorTypes::kRandom};
}

auto create_valid_example_rol_optimization_input() -> input_parser::rol_optimization
{
    constexpr auto tArbitraryMaxIterations = 42;
    constexpr auto tArbitraryStepTolerance = 1e-7;
    constexpr auto tArbitraryGradientTolerance = 1e-5;
    constexpr auto tArbitraryInitialSearchRadius = 15.0;

    return input_parser::rol_optimization{/*.input_file_name=*/boost::none,
                                          /*.export_settings_file_name=*/input_parser::FileName{"output_rol.xml"},
                                          /*.max_iterations =  */ tArbitraryMaxIterations,
                                          /*.step_tolerance = */ tArbitraryStepTolerance,
                                          /*.gradient_tolerance = */ tArbitraryGradientTolerance,
                                          /*.initial_search_radius = */ tArbitraryInitialSearchRadius,
                                          /*.verbose_output = */ false,
                                          /*.approximate_hessian = */ false,
                                          /*.output_design_history = */ false};
}

auto create_valid_example_sensitivity_check_input() -> input_parser::sensitivity_check
{
    return input_parser::sensitivity_check{/*.output_file_name=*/input_parser::FileName{"sensitivity_check.txt"}};
}

}  // namespace plato::process_manager::extension::test_utilities
