#ifndef PLATO_FILTER_EXTENSION_KERNELFILTER
#define PLATO_FILTER_EXTENSION_KERNELFILTER

#include <boost/mpi/communicator.hpp>
#include <filesystem>

#include "plato/filter/extension/LinearMask.hpp"
#include "plato/filter/library/FilterInterface.hpp"
#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/input_parser/InputEnumTypes.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/utilities/NamedType.hpp"

namespace plato::analysis
{
struct AnalysisDomainMesh;
}

// clang-format off
PLATO_FILTER_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), kernel_filter,
    (double, filter_radius, "Required field specifying the size of the filter radius.")
    (plato::input_parser::KernelFilterCenteringTypes, centering_type, "Required field specifying whether the results should be 'node' or 'element' centered. "
                                                                      "Platoanalyze requires node-centered and SD requires element-centered.")
    (bool, use_relative_radius, "Optional field that can convert the filter radius specified into a radius relative to the average element size.")
    (unsigned int, number_of_processors, "Optional parameter that will specify the number of processors used to run the filter in parallel. "
                                         "Requires the 'plato' executable be called with mpirun. Limited to the maximum processors specified in the mpirun call. "
                                         "Over-parallelization can cause a significant degredation of the filter.")
)
// clang-format on

namespace plato::filter::extension
{
using FilterRadius = utilities::NamedType<double, struct FilterRadiusTag>;

/// @brief An implementation of a kernel filter that relies on Tpetra and STK objects to conduct a search and create a
/// linear mask.
// CPD-OFF
class KernelFilter : public library::FilterInterface
{
   public:
    /// @brief Construct a kernel filter object for a mesh @a aMeshFileName, with a filter radius of @a aFilterRadius,
    /// centered on the elements or nodes determined by @a aFilterCentering, using a communicator @a aCommunicator
    KernelFilter(const mesh::Mesh& aMesh,
                 FilterRadius aFilterRadius,
                 input_parser::KernelFilterCenteringTypes aFilterCentering,
                 const boost::mpi::communicator& aCommunicator);

    /// @brief Apply the internal filter to the mesh specified in @a aAnalysisDomainMesh and return a new
    /// AnalysisDomainMesh object
    [[nodiscard]] auto filter(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const
        -> analysis::AnalysisDomainMesh override;

    /// @brief Return the multiplication of row vector @a aV with the Jacobian of the filter.
    [[nodiscard]] auto rowVectorTimesJacobian(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                              const linear_algebra::DynamicVector<double>& aV) const
        -> linear_algebra::DynamicVector<double> override;

    /// @brief Return the multiplication of row vector @a aV with the transpose Jacobian of the filter.
    [[nodiscard]] auto rowVectorTimesAdjointJacobian(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                                     const linear_algebra::DynamicVector<double>& aV) const
        -> linear_algebra::DynamicVector<double> override;

   private:
    LinearMask mLinearMask;
    input_parser::KernelFilterCenteringTypes mFilterCentering;
    boost::mpi::communicator mCommunicator;
};
// CPD-ON

namespace detail
{
[[nodiscard]] auto validate_kernel_filter_centering_type(const input_parser::kernel_filter& aInput)
    -> std::optional<std::string>;

[[nodiscard]] auto validate_number_of_processors(const input_parser::kernel_filter& aInput)
    -> std::optional<std::string>;

[[nodiscard]] auto validate_number_of_processors_factor_of_comm_world(const input_parser::kernel_filter& aInput)
    -> std::optional<std::string>;

/// @brief Create a LinearMask object from mesh @a aMesh, with a filter sphere with radius @a aFilterRadius,
/// centered on the elements or nodes determined by @a aFilterCentering, using a communicator @a aCommunicator
[[nodiscard]] LinearMask create_linear_mask(const mesh::Mesh& aMesh,
                                            FilterRadius aFilterRadius,
                                            input_parser::KernelFilterCenteringTypes aFilterCentering,
                                            const boost::mpi::communicator& aCommunicator);

/// @brief Create a StateCache object for constructing a shared pointer to a KernelFilter if the mesh coordinates have
/// changed (i.e. the mesh has changed)
[[nodiscard]] auto create_filter_cache(const input_parser::kernel_filter& aInput) -> library::FilterCache;

}  // namespace detail
}  // namespace plato::filter::extension

#endif
