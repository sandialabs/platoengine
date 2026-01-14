#ifndef PLATO_FILTER_EXTENSION_KERNELFILTERS_SWEPTFILTER
#define PLATO_FILTER_EXTENSION_KERNELFILTERS_SWEPTFILTER

#include <boost/mpi/communicator.hpp>

#include "plato/filter/extension/kernel_filters/KernelFilterCenteringTypes.hpp"
#include "plato/filter/extension/kernel_filters/KernelFilterInputUtilities.hpp"
#include "plato/filter/extension/kernel_filters/LinearMaskBuilder.hpp"
#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/third_party_integration/common/Vector3.hpp"

namespace plato::analysis
{
struct AnalysisDomainMesh;
}
// CPD-OFF
// clang-format off
PLATO_FILTER_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), z_swept_filter,
    (double, filter_radius, plato::filter::extension::kernel_filters::kKernelFilterRadiusHelpText)
    (plato::input_parser::KernelFilterCenteringTypes, centering_type, plato::filter::extension::kernel_filters::kKernelFilterCenteringTypeHelpText)
    (bool, use_relative_radius, plato::filter::extension::kernel_filters::kKernelFilterRelativeRadiusHelpText)
    (plato::input_parser::FileName, target_mesh_name, "Required field specifying the file name of the 3D exodus mesh to map controls onto using the swept symmetry operation.")
    (unsigned int, number_of_processors, plato::filter::extension::kernel_filters::kKernelFilterNumberOfProcessorsHelpText)
    (plato::input_parser::FixedBlockList, fixed_blocks, plato::filter::extension::kernel_filters::kKernelFilterFixedBlockHelpText)
)
// clang-format on
// CPD-ON
namespace plato::filter::extension::kernel_filters
{
///@brief A specialized filter type based on the z_swept_filter input
using SweptFilterType = KernelFilterFunctions<input_parser::z_swept_filter>;

///@brief Extract the input from @a aInput and @a aAnalysisDomainMesh to create a filter type using the detail function
[[nodiscard]] auto make_z_swept_filter_type(const input_parser::z_swept_filter& aInput,
                                            const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) -> SweptFilterType;

namespace detail
{
///@brief A helper constructor for creating the SweptFilterType needed to construct a SweptFilter. It takes a
/// radius @a aRadius and the centering type @a aFilterCentering and sweep direction @a aDirection. This assigns the
/// compute functions with the proper utilities.
[[nodiscard]] auto make_z_swept_filter_type(const double aRadius,
                                            const input_parser::KernelFilterCenteringTypes aFilterCentering,
                                            const third_party_integration::common::Vector3& aDirection)
    -> SweptFilterType;

///@brief Validation function that checks the mesh @a aSourceMeshFileName to make sure it exists only in the xy plane
[[nodiscard]] auto validate_source_mesh_in_xy_plane(const input_parser::z_swept_filter& aInput,
                                                    const std::filesystem::path& aSourceMeshFileName)
    -> std::optional<std::string>;

}  // namespace detail

}  // namespace plato::filter::extension::kernel_filters

#endif
