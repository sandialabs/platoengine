#include "plato/process_manager/extension/snopt/test_utilities/ExampleInputBlocks.hpp"

namespace plato::process_manager::extension::snopt::test_utilities
{

auto create_valid_example_snopt_optimization_input() -> input_parser::snopt_optimization
{
    return input_parser::snopt_optimization{/*.input_file_name=*/boost::none,
                                            /*.max_iterations=*/10,
                                            /*.time_limit_in_minutes=*/0,
                                            /*.output_design_history=*/false};
}

}  // namespace plato::process_manager::extension::snopt::test_utilities
