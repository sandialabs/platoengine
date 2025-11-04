#include "plato/criteria/extension/SharedLibraryVectorCriterion.hpp"

#include <boost/mpi/communicator.hpp>
#include <string_view>

#include "plato/services/AppConfigurationUtilities.hpp"
#include "plato/services/SharedLibrarySetupTeardown.hpp"
#include "plato/services/SystemLogger.hpp"
#include "plato/services/TaskLogSetupTeardown.hpp"

namespace plato::criteria::extension
{
namespace
{
template <typename FunctionSignature, typename... Args>
auto load_criterion_interface(const services::AppConfigurationWithDirectory& aAppConfiguration,
                              const std::string_view aCreateCriterionFunctionName,
                              Args&&... aArgs)
{
    return std::make_unique<VectorCriterionSharedLibraryObject>(services::make_shared_library_object<FunctionSignature>(
        services::shared_library_path(aAppConfiguration), aCreateCriterionFunctionName, std::forward<Args>(aArgs)...));
}

using SerialFunctionSignature = std::unique_ptr<library::VectorCriterionInterface>(const std::vector<std::string>&);
using ParallelFunctionSignature = std::unique_ptr<library::VectorCriterionInterface>(const std::vector<std::string>&,
                                                                                     MPI_Comm);

/// @brief Writes the components of @a aVector to a string, limiting the number of components to @a aComponentLimit.
[[nodiscard]] auto to_string(const linear_algebra::DynamicVector<double>& aVector,
                             const std::size_t aComponentMax) -> std::string
{
    const auto tComponentLimit = std::min(aComponentMax, aVector.size());
    auto tStream = std::stringstream{};
    std::copy_n(aVector.stdVector().begin(), tComponentLimit, std::ostream_iterator<double>{tStream, " "});
    return tStream.str();
}

}  // namespace

SharedLibraryVectorCriterion::SharedLibraryVectorCriterion(
    const services::AppConfigurationWithDirectory& aAppConfiguration,
    const services::CriterionConfiguration& aCriterionConfiguration,
    const library::CriterionInput& aCriterionInput)
    : mCriterionInterface{load_criterion_interface<SerialFunctionSignature>(
          aAppConfiguration, aCriterionConfiguration.mFunctionName, aCriterionInput.mInputFiles.list().mList)},
      mComponentType{aCriterionInput.mComponentType},
      mName{aCriterionInput.mName}
{
}

SharedLibraryVectorCriterion::SharedLibraryVectorCriterion(
    const services::AppConfigurationWithDirectory& aAppConfiguration,
    const services::CriterionConfiguration& aCriterionConfiguration,
    const library::CriterionInput& aCriterionInput,
    const boost::mpi::communicator& aComm)
    : mCriterionInterface{load_criterion_interface<ParallelFunctionSignature>(
          aAppConfiguration, aCriterionConfiguration.mFunctionName, aCriterionInput.mInputFiles.list().mList, aComm)},
      mComponentType{aCriterionInput.mComponentType},
      mName{aCriterionInput.mName},
      mComm{aComm}
{
}

auto SharedLibraryVectorCriterion::value(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const
    -> linear_algebra::DynamicVector<double>
{
    auto tLogger = services::component_logger(mComponentType, mName);
    tLogger.logInfo("Evaluating vector criterion");

    auto tCriterionValue =
        linear_algebra::DynamicVector<double>(mCriterionInterface->object()->value(aAnalysisDomainMesh));

    constexpr auto tMaxNumberOfComponents = 15U;
    tLogger.logInfo("Evaluation complete, values: \n" + to_string(tCriterionValue, tMaxNumberOfComponents));

    return tCriterionValue;
}

auto SharedLibraryVectorCriterion::rowVectorTimesJacobian(
    const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
    const linear_algebra::DynamicVector<double>& aDirectionVector) const -> linear_algebra::DynamicVector<double>
{
    [[maybe_unused]] const auto tTaskLogger = services::TaskLogSetupTeardown{
        services::jacobian_task_message(), services::component_logger(mComponentType, mName)};

    return linear_algebra::DynamicVector<double>(
        mCriterionInterface->object()->rowVectorTimesJacobian(aAnalysisDomainMesh, aDirectionVector.stdVector()));
}

auto SharedLibraryVectorCriterion::rowVectorTimesAdjointJacobian(
    const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
    const linear_algebra::DynamicVector<double>& aDualVector) const -> linear_algebra::DynamicVector<double>
{
    [[maybe_unused]] const auto tTaskLogger = services::TaskLogSetupTeardown{
        services::adjoint_jacobian_task_message(), services::component_logger(mComponentType, mName)};

    return linear_algebra::DynamicVector<double>(
        mCriterionInterface->object()->rowVectorTimesAdjointJacobian(aAnalysisDomainMesh, aDualVector.stdVector()));
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

auto make_shared_library_vector_function(const SharedLibraryVectorCriterion& aSharedLibCriterion)
    -> library::VectorCriterionFunction
{
    return library::VectorCriterionFunction{
        [aSharedLibCriterion](const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
        { return aSharedLibCriterion.value(aAnalysisDomainMesh); },
        [aSharedLibCriterion](const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
        { return make_jacobian_multiplier(aSharedLibCriterion, aAnalysisDomainMesh); },
        [aSharedLibCriterion](const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) {
            return make_adjoint_jacobian_multiplier(aSharedLibCriterion, aAnalysisDomainMesh);
        }};  // namespace plato::criteria::extension
}

}  // namespace plato::criteria::extension
