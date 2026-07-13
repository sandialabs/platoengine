#ifndef PLATO_FILTER_EXTENSION_KERNELFILTERS_GENERALPLANEREFLECTFILTER
#define PLATO_FILTER_EXTENSION_KERNELFILTERS_GENERALPLANEREFLECTFILTER

#include <boost/mpi/communicator.hpp>

#include "plato/filter/extension/kernel_filters/GeneralPlaneParser.hpp"
#include "plato/filter/extension/kernel_filters/KernelFilterCenteringTypes.hpp"
#include "plato/filter/extension/kernel_filters/KernelFilterInputUtilities.hpp"
#include "plato/filter/extension/kernel_filters/LinearMaskBuilder.hpp"
#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/input_parser/Point.hpp"

namespace plato::analysis
{
struct AnalysisDomainMesh;
}
// CPD-OFF
// clang-format off
PLATO_FILTER_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), general_plane_reflect_filter,
    "In addition to performing distance weighted smoothing, enforces a reflection symmetry through a user-defined plane.",
    (double, filter_radius, plato::filter::extension::kernel_filters::kKernelFilterRadiusHelpText)
    (plato::input_parser::KernelFilterCenteringTypes, centering_type, plato::filter::extension::kernel_filters::kKernelFilterCenteringTypeHelpText)
    (bool, use_relative_radius, plato::filter::extension::kernel_filters::kKernelFilterRelativeRadiusHelpText)
    (plato::input_parser::FileName, target_mesh_name, "Required field specifying the file name of the 3D exodus mesh that the filtered and reflected controls will be mapped onto.")
    (unsigned, number_of_processors, plato::filter::extension::kernel_filters::kKernelFilterNumberOfProcessorsHelpText)
    (plato::input_parser::BlockList, fixed_blocks, plato::filter::extension::kernel_filters::kKernelFilterFixedBlockHelpText)
    (plato::input_parser::GeneralPlane, plane,"Required input defining the plane through which the controls will be reflected. "
                                              "The normal vector does not need to be a unit vector and will be normalized during parsing.")
)
// clang-format on
// CPD-ON
namespace plato::filter::extension::kernel_filters
{
///@brief A specialized filter type based on the general_plane_reflect_filter input
using GeneralPlaneReflectFilterType = KernelFilterAndMeshFunctions<input_parser::general_plane_reflect_filter>;

///@brief Extract the input from @a aInput and @a aAnalysisDomainMesh to create a filter type using the detail function
[[nodiscard]] auto make_general_plane_reflect_filter_type(const input_parser::general_plane_reflect_filter& aInput,
                                                          const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
    -> GeneralPlaneReflectFilterType;

namespace detail
{
///@brief A helper constructor for creating the GeneralPlaneReflectFilterType. It takes a radius @a aRadius and the
/// centering
/// type @a aFilterCentering, and a plane @a aPlane. This assigns the compute functions with the proper utilities.
[[nodiscard]] auto make_general_plane_reflect_filter_type(double aRadius,
                                                          input_parser::KernelFilterCenteringTypes aFilterCentering,
                                                          const Plane& aPlane) -> GeneralPlaneReflectFilterType;

///@brief Validation function that checks the mesh @a aSourceMeshFileName to make sure it exists only on the outward
/// normal side of the plane
[[nodiscard]] auto validate_source_mesh_on_outward_normal_side_of_plane(
    const input_parser::general_plane_reflect_filter& aInput, const std::filesystem::path& aSourceMeshFileName)
    -> std::optional<std::string>;

///@brief Validation function that checks the input @a aInput.plane normal is defined and has nonzero magnitude.
[[nodiscard]] auto validate_required_plane_normal_is_non_zero_magnitude(
    const input_parser::general_plane_reflect_filter& aInput) -> std::optional<std::string>;

///@brief Validation function that checks the input @a aInput.plane is defined.
[[nodiscard]] auto validate_required_plane(const input_parser::general_plane_reflect_filter& aInput)
    -> std::optional<std::string>;

}  // namespace detail

}  // namespace plato::filter::extension::kernel_filters

#endif
