#ifndef PLATO_FILTER_EXTENSION_KERNELFILTERS_KERNELFILTER
#define PLATO_FILTER_EXTENSION_KERNELFILTERS_KERNELFILTER

#include <boost/mpi/communicator.hpp>
#include <functional>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/filter/extension/kernel_filters/KernelFilterCenteringTypes.hpp"
#include "plato/filter/extension/kernel_filters/LinearMask.hpp"
#include "plato/filter/extension/kernel_filters/LinearMaskBuilder.hpp"
#include "plato/filter/library/FilterInterface.hpp"
#include "plato/filter/library/FilterLogger.hpp"
#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/HashGeneration.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/services/TaskLogSetupTeardown.hpp"

namespace plato::filter::extension::kernel_filters
{

template <typename InputParserType>
using KernelFilterTypeMakeFunction =
    std::function<KernelFilterFunctions<InputParserType>(const InputParserType&, const analysis::AnalysisDomainMesh&)>;

/// @brief An implementation of a kernel filter that relies on Tpetra and STK objects to conduct a search and create a
/// linear mask.
// CPD-OFF
template <typename InputParserType>
class KernelFilter : public library::FilterInterface
{
   public:
    /// @brief Construct a kernel filter object for a mesh @a aMeshFileName, with a filter radius of @a aFilterRadius,
    /// centered on the elements or nodes determined by @a aFilterType, using a communicator @a aCommunicator
    KernelFilter(const SourceMesh& aSourceMesh,
                 const TargetMesh& aTargetMesh,
                 const KernelFilterFunctions<InputParserType>& aFilterType,
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
};
// CPD-ON

namespace detail
{

/// @brief Create a LinearMask object from mesh @a aMesh, with a filter sphere with radius @a aFilterRadius,
/// centered on the elements or nodes determined by @a aFilterType, using a communicator @a aCommunicator
template <typename InputParserType>
[[nodiscard]] LinearMask create_linear_mask(const SourceMesh& aSourceMesh,
                                            const TargetMesh& aTargetMesh,
                                            const KernelFilterFunctions<InputParserType>& aFilterType,
                                            const boost::mpi::communicator& aCommunicator);

/// @brief Create a StateCache object for constructing a shared pointer to a KernelFilter if the mesh coordinates have
/// changed (i.e. the mesh has changed)
template <typename InputParserType, typename KernelFilterFunctions>
[[nodiscard]] auto create_kernel_filter_cache(
    const InputParserType& aInput, const KernelFilterTypeMakeFunction<InputParserType>& aKernelFilterTypeMakeFunction)
    -> library::FilterCache;

template <typename InputParserType>
LinearMask create_linear_mask(const SourceMesh& aSourceMesh,
                              const TargetMesh& aTargetMesh,
                              const KernelFilterFunctions<InputParserType>& aFilterType,
                              const boost::mpi::communicator& aCommunicator)
{
    [[maybe_unused]] const auto tTaskLogger =
        services::TaskLogSetupTeardown{"Building filter", library::detail::filter_logger<InputParserType>()};
    return LinearMask{LinearMaskBuilder{SourceMesh{aSourceMesh}, TargetMesh{aTargetMesh}, aFilterType.mFilterCentering,
                                        aFilterType, aCommunicator}
                          .mask(),
                      aCommunicator};
}

template <typename InputParserType>
library::FilterCache create_kernel_filter_cache(
    const InputParserType& aInput, const KernelFilterTypeMakeFunction<InputParserType>& aKernelFilterTypeMakeFunction)
{
    const auto tRequestedRanks = aInput.number_of_processors.value_or(1u);
    const auto tSplitComm = detail::subdivide_world_comm_into_groups(tRequestedRanks);
    return library::FilterCache{
        [aInput, tSplitComm, aKernelFilterTypeMakeFunction](const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
        {
            const auto tFilterType = aKernelFilterTypeMakeFunction(aInput, aAnalysisDomainMesh);
            const auto tTargetMesh = TargetMesh{tFilterType.mTargetMeshFunction(aInput)};
            return std::make_shared<KernelFilter<InputParserType>>(SourceMesh{mesh::Mesh{aAnalysisDomainMesh}},
                                                                   tTargetMesh, tFilterType, tSplitComm);
        },
        [](const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
        { return mesh::hash_mesh_coordinates(aAnalysisDomainMesh); }};
}

}  // namespace detail

template <typename InputParserType>
KernelFilter<InputParserType>::KernelFilter(const SourceMesh& aSourceMesh,
                                            const TargetMesh& aTargetMesh,
                                            const KernelFilterFunctions<InputParserType>& aFilterType,
                                            const boost::mpi::communicator& aCommunicator)
    : mLinearMask{detail::create_linear_mask<InputParserType>(aSourceMesh, aTargetMesh, aFilterType, aCommunicator)},
      mTargetMesh(aTargetMesh),
      mFilterCentering{aFilterType.mFilterCentering},
      mCommunicator{aCommunicator}
{
}

template <typename InputParserType>
analysis::AnalysisDomainMesh KernelFilter<InputParserType>::filter(
    const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const
{
    [[maybe_unused]] const auto tTaskLogger = library::filter_field_task_log<InputParserType>();

    const auto tSourceMesh = mesh::Mesh{aAnalysisDomainMesh};

    const auto tFieldValues =
        mesh::DesignVariablesConversion{tSourceMesh}.analysisDomainMeshToNodalFieldVector(aAnalysisDomainMesh);

    const auto tFilteredField = mLinearMask.matrixMultiply(tFieldValues.mValue);
    if (mFilterCentering == input_parser::KernelFilterCenteringTypes::kNodeCentered)
    {
        return mesh::DesignVariablesConversion{mTargetMesh.mValue}.nodalFieldToAnalysisDomainMesh(
            mesh::NodalFieldVectorReference{std::cref(tFilteredField)});
    }
    return mesh::DesignVariablesConversion{mTargetMesh.mValue}.elementFieldToAnalysisDomainMesh(
        mesh::ElementFieldVectorReference{std::cref(tFilteredField)});
}

template <typename InputParserType>
linear_algebra::DynamicVector<double> KernelFilter<InputParserType>::rowVectorTimesJacobian(
    const analysis::AnalysisDomainMesh& /*aAnalysisDomainMesh*/, const linear_algebra::DynamicVector<double>& aV) const
{
    [[maybe_unused]] const auto tTaskLogger = library::jacobian_task_log<InputParserType>();

    return linear_algebra::DynamicVector<double>{mLinearMask.transposeMatrixMultiply(aV.stdVector())};
}

template <typename InputParserType>
auto KernelFilter<InputParserType>::rowVectorTimesAdjointJacobian(const analysis::AnalysisDomainMesh&,
                                                                  const linear_algebra::DynamicVector<double>& aV) const
    -> linear_algebra::DynamicVector<double>
{
    [[maybe_unused]] const auto tTaskLogger = library::adjoint_jacobian_task_log<InputParserType>();

    return linear_algebra::DynamicVector<double>{mLinearMask.matrixMultiply(aV.stdVector())};
}

}  // namespace plato::filter::extension::kernel_filters

#endif
