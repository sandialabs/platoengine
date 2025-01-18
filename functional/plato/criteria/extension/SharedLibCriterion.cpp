#include "plato/criteria/extension/SharedLibCriterion.hpp"

#include <boost/mpi/communicator.hpp>

#include "plato/services/AppConfigurationUtilities.hpp"
#include "plato/services/SharedLibrarySetupTeardown.hpp"

namespace plato::criteria::extension
{
namespace
{
template <typename FunctionSignature, typename... Args>
auto load_criterion_interface(const services::AppConfigurationWithDirectory& aAppConfiguration,
                              const std::string_view aCreateCriterionFunctionName,
                              Args&&... aArgs)
{
    return std::make_unique<CriterionSharedLibraryObject>(services::make_shared_library_object<FunctionSignature>(
        services::shared_library_path(aAppConfiguration), aCreateCriterionFunctionName, std::forward<Args>(aArgs)...));
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
    return mCriterionInterface->object()->value(aMesh);
}

linear_algebra::DynamicVector<double> SharedLibCriterion::df(const analysis::AnalysisDomainMesh& aAnalysisMesh) const
{
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
