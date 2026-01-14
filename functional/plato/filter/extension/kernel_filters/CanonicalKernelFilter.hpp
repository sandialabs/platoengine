#ifndef PLATO_FILTER_EXTENSION_KERNELFILTERS_CANONICALKERNELFILTER
#define PLATO_FILTER_EXTENSION_KERNELFILTERS_CANONICALKERNELFILTER

#include <boost/mpi/communicator.hpp>

#include "plato/filter/extension/kernel_filters/KernelFilterCenteringTypes.hpp"
#include "plato/filter/extension/kernel_filters/KernelFilterInputUtilities.hpp"
#include "plato/filter/extension/kernel_filters/LinearMaskBuilder.hpp"
#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"

namespace plato::analysis
{
struct AnalysisDomainMesh;
}
// CPD-OFF
// clang-format off
PLATO_FILTER_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), kernel_filter,
    (double, filter_radius, plato::filter::extension::kernel_filters::kKernelFilterRadiusHelpText)
    (plato::input_parser::KernelFilterCenteringTypes, centering_type, plato::filter::extension::kernel_filters::kKernelFilterCenteringTypeHelpText)
    (bool, use_relative_radius, plato::filter::extension::kernel_filters::kKernelFilterRelativeRadiusHelpText)
    (unsigned int, number_of_processors, plato::filter::extension::kernel_filters::kKernelFilterNumberOfProcessorsHelpText)
)
// clang-format on
// CPD-ON
namespace plato::filter::extension::kernel_filters
{
using KernelFilterType = KernelFilterFunctions<input_parser::kernel_filter>;

///@brief Extract the input from @a aInput and @a aAnalysisDomainMesh to create a filter type using the detail function
[[nodiscard]] auto make_kernel_filter_type(const input_parser::kernel_filter& aInput,
                                           const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) -> KernelFilterType;

namespace detail
{
///@brief A helper constructor for creating the KernelFilterType needed to construct a simple KernelFilter. It takes
/// a radius @a aRadius and the centering type @a aFilterCentering. This assigns the compute functions with the proper
/// utilities.
[[nodiscard]] auto make_kernel_filter_type(const double aRadius,
                                           const input_parser::KernelFilterCenteringTypes aFilterCentering,
                                           const mesh::Mesh& aTargetMesh) -> KernelFilterType;
}  // namespace detail

}  // namespace plato::filter::extension::kernel_filters

#endif
