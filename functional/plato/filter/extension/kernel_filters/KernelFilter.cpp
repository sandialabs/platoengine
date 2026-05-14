#include "plato/filter/extension/kernel_filters/KernelFilter.hpp"

#include <boost/mpi/communicator.hpp>
#include <functional>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/filter/extension/kernel_filters/KernelFilterCenteringTypes.hpp"
#include "plato/filter/extension/kernel_filters/LinearMaskBuilder.hpp"
#include "plato/filter/library/FilterLogger.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/services/TaskLogSetupTeardown.hpp"

namespace plato::filter::extension::kernel_filters
{

KernelFilter::KernelFilter(const SourceMesh& aSourceMesh,
                           const TargetMesh& aTargetMesh,
                           const KernelFilterFunctions& aFilterType,
                           const boost::mpi::communicator& aCommunicator)
    : mLinearMask{detail::create_linear_mask(aSourceMesh, aTargetMesh, aFilterType, aCommunicator)},
      mTargetMesh(aTargetMesh),
      mFilterCentering{aFilterType.mFilterCentering},
      mCommunicator{aCommunicator},
      mLoggingName{aFilterType.mLoggingName}
{
}

auto KernelFilter::filter(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const -> analysis::AnalysisDomainMesh
{
    [[maybe_unused]] const auto tTaskLogger = library::filter_field_task_log(mLoggingName);

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

auto KernelFilter::rowVectorTimesJacobian(const analysis::AnalysisDomainMesh& /*aAnalysisDomainMesh*/,
                                          const linear_algebra::DynamicVector<double>& aV) const
    -> linear_algebra::DynamicVector<double>
{
    [[maybe_unused]] const auto tTaskLogger = library::jacobian_task_log(mLoggingName);

    return linear_algebra::DynamicVector<double>{mLinearMask.transposeMatrixMultiply(aV.stdVector())};
}

auto KernelFilter::rowVectorTimesAdjointJacobian(const analysis::AnalysisDomainMesh&,
                                                 const linear_algebra::DynamicVector<double>& aV) const
    -> linear_algebra::DynamicVector<double>
{
    [[maybe_unused]] const auto tTaskLogger = library::adjoint_jacobian_task_log(mLoggingName);

    return linear_algebra::DynamicVector<double>{mLinearMask.matrixMultiply(aV.stdVector())};
}

namespace detail
{
auto create_linear_mask(const SourceMesh& aSourceMesh,
                        const TargetMesh& aTargetMesh,
                        const KernelFilterFunctions& aFilterType,
                        const boost::mpi::communicator& aCommunicator) -> LinearMask
{
    [[maybe_unused]] const auto tTaskLogger =
        services::TaskLogSetupTeardown{"Building filter", library::detail::filter_logger(aFilterType.mLoggingName)};
    return LinearMask{
        LinearMaskBuilder{SourceMesh{aSourceMesh}, TargetMesh{aTargetMesh}, aFilterType, aCommunicator}.mask(),
        aCommunicator};
}
}  // namespace detail

}  // namespace plato::filter::extension::kernel_filters
