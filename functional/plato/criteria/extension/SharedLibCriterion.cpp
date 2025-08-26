#include "plato/criteria/extension/SharedLibCriterion.hpp"

#include <boost/mpi/communicator.hpp>

#include "plato/services/AppConfigurationUtilities.hpp"
#include "plato/services/ScopedExternalRedirectLogger.hpp"
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
                              const library::CriterionInput& aCriterionInput,
                              Args&&... aArgs)
{
    [[maybe_unused]] const auto tScopedLogger =
        services::ScopedExternalRedirectLogger{aCriterionInput.mComponentType, aCriterionInput.mName};

    return std::make_unique<CriterionSharedLibraryObject>(services::make_shared_library_object<FunctionSignature>(
        services::shared_library_path(aAppConfiguration), aCreateCriterionFunctionName, std::forward<Args>(aArgs)...));
}

using SerialFunctionSignature = std::unique_ptr<library::CriterionInterface>(const std::vector<std::string>&);
using ParallelFunctionSignature = std::unique_ptr<library::CriterionInterface>(const std::vector<std::string>&,
                                                                               MPI_Comm);

}  // namespace

SharedLibCriterion::SharedLibCriterion(const services::AppConfigurationWithDirectory& aAppConfiguration,
                                       const services::CriterionConfiguration& aCriterionConfiguration,
                                       const library::CriterionInput& aCriterionInput)
    : mCriterionInterface{load_criterion_interface<SerialFunctionSignature>(aAppConfiguration,
                                                                            aCriterionConfiguration.mFunctionName,
                                                                            aCriterionInput,
                                                                            aCriterionInput.mInputFiles.list().mList)},
      mComponentType{aCriterionInput.mComponentType},
      mName{aCriterionInput.mName}
{
}

SharedLibCriterion::SharedLibCriterion(const services::AppConfigurationWithDirectory& aAppConfiguration,
                                       const services::CriterionConfiguration& aCriterionConfiguration,
                                       const library::CriterionInput& aCriterionInput,
                                       const boost::mpi::communicator& aComm)
    : mCriterionInterface{load_criterion_interface<ParallelFunctionSignature>(aAppConfiguration,
                                                                              aCriterionConfiguration.mFunctionName,
                                                                              aCriterionInput,
                                                                              aCriterionInput.mInputFiles.list().mList,
                                                                              aComm)},
      mComm{aComm},
      mComponentType{aCriterionInput.mComponentType},
      mName{aCriterionInput.mName}
{
}

double SharedLibCriterion::f(const analysis::AnalysisDomainMesh& aMesh) const
{
    auto tLogger = services::component_logger(mComponentType, mName);
    tLogger.logInfo("Evaluating criterion");

    const auto tValue = [&aMesh, this]()
    {
        [[maybe_unused]] const auto tScopedLogger = services::ScopedExternalRedirectLogger{mComponentType, mName};
        return mCriterionInterface->object()->value(aMesh);
    }();

    tLogger.logInfo("Evaluation complete. Criterion value = " + std::to_string(tValue));
    return tValue;
}

linear_algebra::DynamicVector<double> SharedLibCriterion::df(const analysis::AnalysisDomainMesh& aAnalysisMesh) const
{
    [[maybe_unused]] const auto tTaskLogger =
        services::TaskLogSetupTeardown{"Gradient", services::component_logger(mComponentType, mName)};

    return linear_algebra::DynamicVector<double>(mCriterionInterface->object()->gradient(aAnalysisMesh));
}

auto make_shared_lib_function(const SharedLibCriterion& aSharedLibCriterion) -> library::CriterionFunction
{
    return core::make_function_with_first_derivative(
        [aSharedLibCriterion](const analysis::AnalysisDomainMesh& aAnalysisMesh)
        { return aSharedLibCriterion.f(aAnalysisMesh); },
        [aSharedLibCriterion](const analysis::AnalysisDomainMesh& aAnalysisMesh)
        { return aSharedLibCriterion.df(aAnalysisMesh); });
}

}  // namespace plato::criteria::extension
