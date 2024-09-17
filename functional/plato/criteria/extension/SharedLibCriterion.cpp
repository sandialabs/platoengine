#include "plato/criteria/extension/SharedLibCriterion.hpp"

#include <boost/mpi/communicator.hpp>

#include "plato/services/AppConfigurationUtilities.hpp"
#include "plato/services/SharedLibrarySetupTeardown.hpp"

namespace plato::criteria::extension
{
namespace
{
template <typename FunctionSignature, typename... Args>
std::unique_ptr<library::CriterionInterface> load_criterion_interface(
    const services::AppConfigurationWithDirectory& aAppConfiguration,
    const std::string_view aCreateCriterionFunctionName,
    Args&&... aArgs)
{
    auto tSharedLibrary = services::SharedLibrarySetupTeardown{services::shared_library_path(aAppConfiguration)};
    return tSharedLibrary.call<FunctionSignature>(aCreateCriterionFunctionName, std::forward<Args>(aArgs)...);
}

using SerialFunctionSignature = std::unique_ptr<library::CriterionInterface>(const std::vector<std::string>&);
using ParallelFunctionSignature = std::unique_ptr<library::CriterionInterface>(const std::vector<std::string>&,
                                                                               MPI_Comm);

}  // namespace

SharedLibCriterion::SharedLibCriterion(const services::AppConfigurationWithDirectory& aAppConfiguration,
                                       const services::CriterionConfiguration& aCriterionConfiguration,
                                       const std::vector<std::string>& aFileNames)
    : mCriterionInterface{load_criterion_interface<SerialFunctionSignature>(
          aAppConfiguration, aCriterionConfiguration.mFunctionName, aFileNames)}
{
}

SharedLibCriterion::SharedLibCriterion(const services::AppConfigurationWithDirectory& aAppConfiguration,
                                       const services::CriterionConfiguration& aCriterionConfiguration,
                                       const std::vector<std::string>& aFileNames,
                                       const boost::mpi::communicator& aComm)
    : mCriterionInterface{load_criterion_interface<ParallelFunctionSignature>(
          aAppConfiguration, aCriterionConfiguration.mFunctionName, aFileNames, aComm)},
      mComm{aComm}
{
}

double SharedLibCriterion::f(const analysis::AnalysisDomainMesh& aMesh) const
{
    return mCriterionInterface->value(aMesh);
}

linear_algebra::DynamicVector<double> SharedLibCriterion::df(const analysis::AnalysisDomainMesh& aAnalysisMesh) const
{
    return mCriterionInterface->value(aAnalysisMesh);
}

linear_algebra::DynamicVector<double> SharedLibCriterion::df(const analysis::AnalysisDomainMesh& aAnalysisMesh) const
{
    return linear_algebra::DynamicVector<double>(mCriterionInterface->gradient(aAnalysisMesh));
}

auto make_shared_lib_function(const SharedLibCriterion& aSharedLibCriterion)
    -> core::Function<double, linear_algebra::DynamicVector<double>, const analysis::AnalysisDomainMesh&>
{
    return core::make_function([aSharedLibCriterion](const analysis::AnalysisDomainMesh& aAnalysisMesh)
                               { return aSharedLibCriterion.f(aAnalysisMesh); },
                               [aSharedLibCriterion](const analysis::AnalysisDomainMesh& aAnalysisMesh)
                               { return aSharedLibCriterion.df(aAnalysisMesh); });
}

}  // namespace plato::criteria::extension
