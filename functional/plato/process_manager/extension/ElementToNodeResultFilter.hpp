#ifndef PLATO_PROCESS_MANAGER_EXTENSION_ELEMENTTONODERESULTFILTER
#define PLATO_PROCESS_MANAGER_EXTENSION_ELEMENTTONODERESULTFILTER

#include "plato/input_parser/CrossReference.hpp"
#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/process_manager/library/ProcessManagerRegistration.hpp"

// clang-format off
PLATO_PROCESS_MANAGER_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), element_to_node_result_filter,
    (plato::input_parser::CrossReference<plato::input_parser::ComponentType::kFilter>, filter, "Name of the filter to apply. "
                                                                                               "Only required if more than one filter is specified.")
    (plato::input_parser::CrossReference<plato::input_parser::ComponentType::kGeometry>, geometry, "Name of the geometry whose output mesh will be used. "
                                                                                                   "Only required if more than one geometry is specified.")
    (plato::input_parser::FileName, output_file_name, "Optional: If omitted, the output file from density_topology will be used.")
)
// clang-format on

namespace plato::process_manager::library
{
struct ProcessManagerData;
}

namespace plato::process_manager::extension
{

/// @brief The purpose of this process manager is to convert an element-based filter result from a density topology
/// run to a nodal result.
///
/// Element-based fields can be difficult to post-process in some visualization software and so this enables users
/// to generate a nodal field result if desired.
class ElementToNodeResultFilter
{
   public:
    explicit ElementToNodeResultFilter(const library::ValidatedProcessManagerInput& aInput);

    void run(const library::ProcessManagerData& aProcessManagerData) const;

   private:
};

namespace detail
{
[[nodiscard]] auto validate_filter_is_kernel_filter(const input_parser::element_to_node_result_filter& aInput)
    -> std::optional<std::string>;

[[nodiscard]] auto validate_has_mesh_name_or_geometry_is_density_topology(
    const input_parser::element_to_node_result_filter& aInput) -> std::optional<std::string>;
}  // namespace detail

}  // namespace plato::process_manager::extension

#endif
