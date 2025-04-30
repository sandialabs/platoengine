#include "plato/filter/extension/test_utilities/ExampleInputBlocks.hpp"

namespace plato::filter::extension::test_utilities
{
auto create_valid_helmholtz_filter_input() -> input_parser::helmholtz_filter
{
    return input_parser::helmholtz_filter{/*.filter_radius=*/91.0,
                                          /*.use_relative_radius=*/boost::none,
                                          /*.boundary_sticking_penalty=*/1.0};
}

auto create_valid_identity_filter_input() -> input_parser::identity_filter
{
    return input_parser::identity_filter{/*.filter_radius = */ boost::none};
}

auto create_valid_kernel_filter_input() -> input_parser::kernel_filter
{
    return input_parser::kernel_filter{/*.filter_radius=*/17.0,
                                       /*.centering_type=*/input_parser::KernelFilterCenteringTypes::kNodeCentered,
                                       /*.use_relative_radius=*/boost::none,
                                       /*.number_of_processors*/ 1};
}

}  // namespace plato::filter::extension::test_utilities
