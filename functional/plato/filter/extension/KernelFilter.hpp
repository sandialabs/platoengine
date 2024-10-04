#ifndef PLATO_FILTER_EXTENSION_KERNELFILTER
#define PLATO_FILTER_EXTENSION_KERNELFILTER

#include <boost/mpi/communicator.hpp>
#include <filesystem>

#include "plato/filter/extension/LinearMask.hpp"
#include "plato/filter/library/FilterInterface.hpp"
#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/input_parser/InputEnumTypes.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/utilities/NamedType.hpp"

namespace plato::analysis
{
struct AnalysisDomainMesh;
}

namespace plato::input_parser
{
struct kernel_filter;
}

namespace plato::filter::extension
{
using FilterRadius = utilities::NamedType<double, struct FilterRadiusTag>;

/// @brief An implementation of a kernel filter that relies on Tpetra and STK objects to conduct a search and create a
/// linear mask.
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
    [[nodiscard]] analysis::AnalysisDomainMesh filter(
        const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const override;

    /// @brief Return the Jacobian of the linear mask applied to a specific vector @a aV
    [[nodiscard]] linear_algebra::DynamicVector<double> rowVectorTimesJacobian(
        const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
        const linear_algebra::DynamicVector<double>& aV) const override;

   private:
    LinearMask mLinearMask;
    input_parser::KernelFilterCenteringTypes mFilterCentering;
    boost::mpi::communicator mCommunicator;
};

namespace detail
{
[[nodiscard]] std::optional<std::string> validate_kernel_filter_centering_type(
    const input_parser::kernel_filter& aInput);

[[nodiscard]] std::optional<std::string> validate_number_of_processors(const input_parser::kernel_filter& aInput);

[[nodiscard]] std::optional<std::string> validate_number_of_processors_factor_of_comm_world(
    const input_parser::kernel_filter& aInput);

/// @brief Create a LinearMask object from mesh @a aMesh, with a filter sphere with radius @a aFilterRadius,
/// centered on the elements or nodes determined by @a aFilterCentering, using a communicator @a aCommunicator
[[nodiscard]] LinearMask create_linear_mask(const mesh::Mesh& aMesh,
                                            FilterRadius aFilterRadius,
                                            input_parser::KernelFilterCenteringTypes aFilterCentering,
                                            const boost::mpi::communicator& aCommunicator);

/// @brief Create a StateCache object for constructing a shared pointer to a KernelFilter if the mesh coordinates have
/// changed (i.e. the mesh has changed)
[[nodiscard]] library::FilterCache create_filter_cache(const input_parser::kernel_filter& aInput);

}  // namespace detail
}  // namespace plato::filter::extension

#endif
