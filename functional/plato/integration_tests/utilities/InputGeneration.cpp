#include "plato/integration_tests/utilities/InputGeneration.hpp"

#include "plato/criteria/library/test_utilities/ExampleInputBlocks.hpp"
#include "plato/filter/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/geometry/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/process_manager/extension/test_utilities/ExampleInputBlocks.hpp"

namespace plato::integration_tests::utilities
{
namespace
{
[[nodiscard]] auto create_valid_element_centered_kernel_filter() -> input_parser::kernel_filter
{
    return input_parser::kernel_filter{/*.filter_radius=*/2.0,
                                       /*.centering_type=*/input_parser::KernelFilterCenteringTypes::kElementCentered,
                                       /*.use_relative_radius=*/false,
                                       /*.number_of_processors*/ 1U};
}

}  // namespace

auto create_valid_example_input() -> input_parser::ParsedInput
{
    return geometry::extension::test_utilities::create_valid_density_topology_geometry_input() |
           filter::extension::test_utilities::create_valid_identity_filter_input() |
           criteria::library::test_utilities::create_valid_example_constraint_input() |
           criteria::library::test_utilities::create_valid_example_objective_input() |
           process_manager::extension::test_utilities::create_valid_example_rol_optimization_input();
}

auto create_valid_density_topology_geometry_with_element_centered_kernel_filter_input(
    const std::filesystem::path& aMeshPath) -> input_parser::ParsedInput
{
    auto tGeometry = geometry::extension::test_utilities::create_valid_density_topology_geometry_input();
    tGeometry.mesh_name = input_parser::FileName{aMeshPath.string()};
    return tGeometry | create_valid_element_centered_kernel_filter();
}

}  // namespace plato::integration_tests::utilities
