#include "plato/criteria/extension/SharedLibraryVectorCriterion.hpp"

#include <boost/mpi/communicator.hpp>
#include <string_view>

#include "plato/services/AppConfigurationUtilities.hpp"
#include "plato/services/SharedLibrarySetupTeardown.hpp"

namespace plato::criteria::extension
{
namespace
{
template <typename FunctionSignature, typename... Args>
std::unique_ptr<library::VectorCriterionInterface> load_criterion_interface(
    const services::AppConfigurationWithDirectory& aAppConfiguration,
    const std::string_view aCreateCriterionFunctionName,
    Args&&... aArgs)
{
    auto tSharedLibrary = services::SharedLibrarySetupTeardown{services::shared_library_path(aAppConfiguration)};
    return tSharedLibrary.call<FunctionSignature>(aCreateCriterionFunctionName, std::forward<Args>(aArgs)...);
}

using SerialFunctionSignature = std::unique_ptr<library::VectorCriterionInterface>(const std::vector<std::string>&);
using ParallelFunctionSignature = std::unique_ptr<library::VectorCriterionInterface>(const std::vector<std::string>&,
                                                                                     MPI_Comm);

}  // namespace

SharedLibraryVectorCriterion::SharedLibraryVectorCriterion(
    const services::AppConfigurationWithDirectory& aAppConfiguration,
    const services::CriterionConfiguration& aCriterionConfiguration,
    const std::vector<std::string>& aFileNames)
    : mCriterionInterface{load_criterion_interface<SerialFunctionSignature>(
          aAppConfiguration, aCriterionConfiguration.mFunctionName, aFileNames)}
{
}

SharedLibraryVectorCriterion::SharedLibraryVectorCriterion(
    const services::AppConfigurationWithDirectory& aAppConfiguration,
    const services::CriterionConfiguration& aCriterionConfiguration,
    const std::vector<std::string>& aFileNames,
    const boost::mpi::communicator& aComm)
    : mCriterionInterface{load_criterion_interface<ParallelFunctionSignature>(
          aAppConfiguration, aCriterionConfiguration.mFunctionName, aFileNames, aComm)},
      mComm{aComm}
{
}

auto SharedLibraryVectorCriterion::value(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const
    -> linear_algebra::DynamicVector<double>
{
    return linear_algebra::DynamicVector<double>(mCriterionInterface->value(aAnalysisDomainMesh));
}

auto SharedLibraryVectorCriterion::rowVectorTimesJacobian(
    const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
    const linear_algebra::DynamicVector<double>& aDirectionVector) const -> linear_algebra::DynamicVector<double>
{
    return linear_algebra::DynamicVector<double>(
        mCriterionInterface->rowVectorTimesJacobian(aAnalysisDomainMesh, aDirectionVector.stdVector()));
}

auto SharedLibraryVectorCriterion::rowVectorTimesAdjointJacobian(
    const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
    const linear_algebra::DynamicVector<double>& aDualVector) const -> linear_algebra::DynamicVector<double>
{
    return linear_algebra::DynamicVector<double>(
        mCriterionInterface->rowVectorTimesAdjointJacobian(aAnalysisDomainMesh, aDualVector.stdVector()));
}

namespace
{
auto make_jacobian_multiplier(const SharedLibraryVectorCriterion& aSharedLibCriterion,
                              const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
    -> linear_algebra::JacobianMultiplier
{
    return linear_algebra::JacobianMultiplier{[aSharedLibCriterion, aAnalysisDomainMesh](const auto aDirectionVector) {
        return aSharedLibCriterion.rowVectorTimesJacobian(aAnalysisDomainMesh, aDirectionVector);
    }};
}

auto make_adjoint_jacobian_multiplier(const SharedLibraryVectorCriterion& aSharedLibCriterion,
                                      const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
    -> linear_algebra::AdjointJacobianMultiplier
{
    return linear_algebra::AdjointJacobianMultiplier{linear_algebra::JacobianMultiplier{
        [aSharedLibCriterion, aAnalysisDomainMesh](const auto aDualVector)
        { return aSharedLibCriterion.rowVectorTimesAdjointJacobian(aAnalysisDomainMesh, aDualVector); }}};
}

}  // namespace

auto make_shared_library_jacobian_function(const SharedLibraryVectorCriterion& aSharedLibCriterion)
    -> SharedLibraryVectorCriterionFunction
{
    return SharedLibraryVectorCriterionFunction{
        [aSharedLibCriterion](const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
        { return aSharedLibCriterion.value(aAnalysisDomainMesh); },
        [aSharedLibCriterion](const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
        { return make_jacobian_multiplier(aSharedLibCriterion, aAnalysisDomainMesh); },
        [aSharedLibCriterion](const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) {
            return make_adjoint_jacobian_multiplier(aSharedLibCriterion, aAnalysisDomainMesh);
        }};  // namespace plato::criteria::extension
}

}  // namespace plato::criteria::extension
