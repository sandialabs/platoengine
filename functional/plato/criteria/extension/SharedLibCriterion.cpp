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

double SharedLibCriterion::f(const core::MeshProxy& aMesh) const { return mCriterionInterface->value(aMesh); }

linear_algebra::DynamicVector<double> SharedLibCriterion::df(const core::MeshProxy& aMesh) const
{
    return linear_algebra::DynamicVector<double>(mCriterionInterface->gradient(aMesh));
}

auto make_shared_lib_function(const SharedLibCriterion& aSharedLibCriterion)
    -> core::Function<double, linear_algebra::DynamicVector<double>, const core::MeshProxy&>
{
    return core::make_function(
        [aSharedLibCriterion](const core::MeshProxy& mesh) { return aSharedLibCriterion.f(mesh); },
        [aSharedLibCriterion](const core::MeshProxy& mesh) { return aSharedLibCriterion.df(mesh); });
}

}  // namespace plato::criteria::extension
