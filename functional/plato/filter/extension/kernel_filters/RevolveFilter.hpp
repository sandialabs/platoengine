#ifndef PLATO_FILTER_EXTENSION_KERNELFILTERS_REVOLVEFILTER
#define PLATO_FILTER_EXTENSION_KERNELFILTERS_REVOLVEFILTER

#include <boost/mpi/communicator.hpp>
#include <optional>
#include <string>

#include "plato/filter/extension/kernel_filters/KernelFilterCenteringTypes.hpp"
#include "plato/filter/extension/kernel_filters/KernelFilterInputUtilities.hpp"
#include "plato/filter/extension/kernel_filters/LinearMaskBuilder.hpp"
#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"

namespace plato::analysis
{
struct AnalysisDomainMesh;
}
// CPD-OFF
// clang-format off
PLATO_FILTER_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), y_axis_revolve_filter,
    (double, filter_radius, plato::filter::extension::kernel_filters::kKernelFilterRadiusHelpText)
    (plato::input_parser::KernelFilterCenteringTypes, centering_type, plato::filter::extension::kernel_filters::kKernelFilterCenteringTypeHelpText)
    (bool, use_relative_radius, plato::filter::extension::kernel_filters::kKernelFilterRelativeRadiusHelpText)
    (plato::input_parser::FileName, target_mesh_name, "Required field specifying the file name of the 3D exodus mesh to map controls onto using the revolve symmetry operation.")
    (unsigned int, number_of_processors, plato::filter::extension::kernel_filters::kKernelFilterNumberOfProcessorsHelpText)
    (plato::input_parser::BlockList, fixed_blocks, plato::filter::extension::kernel_filters::kKernelFilterFixedBlockHelpText)
)
// clang-format on
// CPD-ON
namespace plato::filter::extension::kernel_filters
{
///@brief A specialized filter type based on the y_axis_revolve_filter input
using RevolveFilterType = KernelFilterFunctions<input_parser::y_axis_revolve_filter>;

///@brief Extract the input from @a aInput and @a aAnalysisDomainMesh to create a filter type using the detail function
[[nodiscard]] auto make_y_axis_revolve_filter_type(const input_parser::y_axis_revolve_filter& aInput,
                                                   const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
    -> RevolveFilterType;

namespace detail
{
///@brief A helper constructor for creating the RevolveFilterType needed to construct a RevolveFilter. It takes a
/// radius @a aRadius and the centering type @a aFilterCentering. This assigns the
/// compute functions with the proper utilities.
[[nodiscard]] auto make_y_axis_revolve_filter_type(const double aRadius,
                                                   const input_parser::KernelFilterCenteringTypes aFilterCentering)
    -> RevolveFilterType;

///@brief Validation function that checks the source mesh @a aSourceMeshFileName to make sure it exists only in the xz
/// plane
[[nodiscard]] auto validate_source_mesh_in_xz_plane(const input_parser::y_axis_revolve_filter& aInput,
                                                    const std::filesystem::path& aSourceMeshFileName)
    -> std::optional<std::string>;

}  // namespace detail

}  // namespace plato::filter::extension::kernel_filters

#endif
