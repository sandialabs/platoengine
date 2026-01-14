#ifndef PLATO_FILTER_EXTENSION_KERNELFILTERS_WEDGEFILTER
#define PLATO_FILTER_EXTENSION_KERNELFILTERS_WEDGEFILTER

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
    (plato)(input_parser), wedge_filter,
    (double, filter_radius, plato::filter::extension::kernel_filters::kKernelFilterRadiusHelpText)
    (plato::input_parser::KernelFilterCenteringTypes, centering_type, plato::filter::extension::kernel_filters::kKernelFilterCenteringTypeHelpText)
    (bool, use_relative_radius, plato::filter::extension::kernel_filters::kKernelFilterRelativeRadiusHelpText)
    (plato::input_parser::FileName, target_mesh_name, "Required field specifying the file name of the 3D exodus mesh to map controls onto using the wedge symmetry operation.")
    (unsigned int, number_of_processors, plato::filter::extension::kernel_filters::kKernelFilterNumberOfProcessorsHelpText)
    (double, wedge_angle, "Required field specifying the angle in degrees of the wedge (the space between the x-plane and a rotated x-plane).")
    (plato::input_parser::FixedBlockList, fixed_blocks, plato::filter::extension::kernel_filters::kKernelFilterFixedBlockHelpText)
)
// clang-format on
// CPD-ON
namespace plato::filter::extension::kernel_filters
{
///@brief A specialized filter type based on the wedge_filter input
using WedgeFilterType = KernelFilterFunctions<input_parser::wedge_filter>;

///@brief Extract the input from @a aInput and @a aAnalysisDomainMesh to create a filter type using the detail function
[[nodiscard]] auto make_wedge_filter_type(const input_parser::wedge_filter& aInput,
                                          const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) -> WedgeFilterType;

namespace detail
{
///@brief A helper constructor for creating the WedgeFilterType. It takes a radius @a aRadius and the centering type
///@a aFilterCentering and a @a aWedgeAngle. This assigns the compute functions with the proper utilities.
[[nodiscard]] auto make_wedge_filter_type(const double aRadius,
                                          const input_parser::KernelFilterCenteringTypes aFilterCentering,
                                          const double aWedgeAngle) -> WedgeFilterType;

///@brief Validate that the wedge angle is specified in @a aInput and that it evenly divides 360.
[[nodiscard]] auto validate_wedge_angle_commensurate_with_360(const input_parser::wedge_filter& aInput)
    -> std::optional<std::string>;

///@brief Validate that the source mesh @a aSourceMeshFileName has nodes within the wedge angle is specified in @a
/// aInput.
[[nodiscard]] auto validate_source_mesh_matches_wedge_angle(const input_parser::wedge_filter& aInput,
                                                            const std::filesystem::path& aSourceMeshFileName)
    -> std::optional<std::string>;

///@brief Validate that the source mesh @a aSourceMeshFileName has positive y values on the nodes.
[[nodiscard]] auto validate_source_mesh_wedge_positive_y(const input_parser::wedge_filter& aInput,
                                                         const std::filesystem::path& aSourceMeshFileName)
    -> std::optional<std::string>;

}  // namespace detail

}  // namespace plato::filter::extension::kernel_filters

#endif
