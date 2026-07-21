#include "plato/criteria/library/SharedLibCriterion.hpp"

#include <mpi.h>

#include <boost/mpi/communicator.hpp>
#include <memory>
#include <string_view>
#include <vector>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/core/Function.hpp"
#include "plato/criteria/library/CriterionInterface.hpp"
#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/services/AppConfiguration.hpp"
#include "plato/services/AppConfigurationUtilities.hpp"
#include "plato/services/ScopedExternalRedirectLogger.hpp"
#include "plato/services/SharedLibrarySetupTeardown.hpp"

namespace plato::criteria::library
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
        services::shared_library_data(aAppConfiguration), aCreateCriterionFunctionName, std::forward<Args>(aArgs)...));
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
                                       const boost::mpi::communicator& aCommunicator)
    : mCriterionInterface{load_criterion_interface<ParallelFunctionSignature>(aAppConfiguration,
                                                                              aCriterionConfiguration.mFunctionName,
                                                                              aCriterionInput,
                                                                              aCriterionInput.mInputFiles.list().mList,
                                                                              aCommunicator)},
      mComm{aCommunicator},
      mComponentType{aCriterionInput.mComponentType},
      mName{aCriterionInput.mName}
{
}

auto SharedLibCriterion::f(const analysis::AnalysisDomainMesh& aMesh) const -> double
{
    [[maybe_unused]] const auto tScopedLogger = services::ScopedExternalRedirectLogger{mComponentType, mName};
    return mCriterionInterface->object()->value(aMesh);
}

auto SharedLibCriterion::df(const analysis::AnalysisDomainMesh& aMesh) const -> linear_algebra::DynamicVector<double>
{
    [[maybe_unused]] const auto tScopedLogger = services::ScopedExternalRedirectLogger{mComponentType, mName};
    return linear_algebra::DynamicVector<double>(mCriterionInterface->object()->gradient(aMesh));
}

auto make_shared_lib_function(const SharedLibCriterion& aSharedLibCriterion) -> library::CriterionFunction
{
    return core::make_function_with_first_derivative(
        [aSharedLibCriterion](const analysis::AnalysisDomainMesh& aAnalysisMesh)
        { return aSharedLibCriterion.f(aAnalysisMesh); },
        [aSharedLibCriterion](const analysis::AnalysisDomainMesh& aAnalysisMesh)
        { return aSharedLibCriterion.df(aAnalysisMesh); });
}

}  // namespace plato::criteria::library
