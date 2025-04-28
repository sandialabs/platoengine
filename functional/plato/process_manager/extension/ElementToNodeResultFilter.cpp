#include "plato/process_manager/extension/ElementToNodeResultFilter.hpp"

#include "plato/filter/extension/KernelFilter.hpp"
#include "plato/geometry/extension/DensityTopology.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"

namespace plato::process_manager::extension
{

namespace detail
{
auto validate_filter_is_kernel_filter(const input_parser::element_to_node_result_filter& aInput)
    -> std::optional<std::string>
{
    if (aInput.filter && !aInput.filter->mInputBlock.holdsExpectedType<input_parser::kernel_filter>())
    {
        return std::optional<std::string>{
            input_parser::block_name<input_parser::element_to_node_result_filter>() +
            " requires that a kernel_filter is used. The filter type is: " + aInput.filter->mName};
    }
    return {};
}

auto validate_has_mesh_name_or_geometry_is_density_topology(const input_parser::element_to_node_result_filter& aInput)
    -> std::optional<std::string>
{
    if (aInput.geometry && !aInput.geometry->mInputBlock.holdsExpectedType<input_parser::density_topology>() &&
        !aInput.output_file_name)
    {
        return std::optional<std::string>{
            input_parser::block_name<input_parser::element_to_node_result_filter>() +
            " requires that a density_topology component be used if no output_file_name is provided."};
    }
    return {};
}

}  // namespace detail
}  // namespace plato::process_manager::extension
