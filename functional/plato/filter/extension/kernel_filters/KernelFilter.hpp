#ifndef PLATO_FILTER_EXTENSION_KERNELFILTERS_KERNELFILTER
#define PLATO_FILTER_EXTENSION_KERNELFILTERS_KERNELFILTER

#include <boost/mpi/communicator.hpp>
#include <functional>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/filter/extension/kernel_filters/KernelFilterCenteringTypes.hpp"
#include "plato/filter/extension/kernel_filters/LinearMask.hpp"
#include "plato/filter/extension/kernel_filters/LinearMaskBuilder.hpp"
#include "plato/filter/library/FilterInterface.hpp"
#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/mesh/HashGeneration.hpp"
#include "plato/mesh/Mesh.hpp"

namespace plato::filter::extension::kernel_filters
{

template <typename InputParserType>
using KernelFilterTypeMakeFunction = std::function<KernelFilterAndMeshFunctions<InputParserType>(
    const InputParserType&, const analysis::AnalysisDomainMesh&)>;

/// @brief An implementation of a kernel filter that relies on Tpetra and STK objects to conduct a search and create a
/// linear mask.
// CPD-OFF
class KernelFilter : public library::FilterInterface
{
   public:
    /// @brief Construct a kernel filter object for a mesh @a aMeshFileName, with a filter radius of @a aFilterRadius,
    /// centered on the elements or nodes determined by @a aFilterType, using a communicator @a aCommunicator
    KernelFilter(const SourceMesh& aSourceMesh,
                 const TargetMesh& aTargetMesh,
                 const KernelFilterFunctions& aFilterType,
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
    TargetMesh mTargetMesh;
    input_parser::KernelFilterCenteringTypes mFilterCentering;
    boost::mpi::communicator mCommunicator;
    std::string mLoggingName;
};
// CPD-ON

namespace detail
{

/// @brief Create a LinearMask object from mesh @a aMesh, with a filter sphere with radius @a aFilterRadius,
/// centered on the elements or nodes determined by @a aFilterType, using a communicator @a aCommunicator
[[nodiscard]] auto create_linear_mask(const SourceMesh& aSourceMesh,
                                      const TargetMesh& aTargetMesh,
                                      const KernelFilterFunctions& aFilterType,
                                      const boost::mpi::communicator& aCommunicator) -> LinearMask;

/// @brief Create a StateCache object for constructing a shared pointer to a KernelFilter if the mesh coordinates have
/// changed (i.e. the mesh has changed)
template <typename InputParserType>
[[nodiscard]] auto create_kernel_filter_cache(
    const InputParserType& aInput,
    const KernelFilterTypeMakeFunction<InputParserType>& aKernelFilterTypeMakeFunction) -> library::FilterCache;

template <typename InputParserType>
auto create_kernel_filter_cache(const InputParserType& aInput,
                                const KernelFilterTypeMakeFunction<InputParserType>& aKernelFilterTypeMakeFunction)
    -> library::FilterCache
{
    const auto tRequestedRanks = aInput.number_of_processors.value_or(1U);
    const auto tSplitComm = detail::subdivide_world_comm_into_groups(tRequestedRanks);
    return library::FilterCache{
        [aInput, tSplitComm, aKernelFilterTypeMakeFunction](const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
        {
            const auto tFilterType = aKernelFilterTypeMakeFunction(aInput, aAnalysisDomainMesh);
            const auto tTargetMesh = TargetMesh{tFilterType.mTargetMeshFunction(aInput)};
            return std::make_shared<KernelFilter>(SourceMesh{mesh::Mesh{aAnalysisDomainMesh}}, tTargetMesh,
                                                  tFilterType.mKernelFilterFunctions, tSplitComm);
        },
        [](const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
        { return mesh::hash_mesh_coordinates(aAnalysisDomainMesh); }};
}

}  // namespace detail

}  // namespace plato::filter::extension::kernel_filters

#endif
