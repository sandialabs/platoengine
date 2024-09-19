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

auto SharedLibraryVectorCriterion::value(const mesh::MeshDesignVariables& aMeshDesignVariables) const
    -> linear_algebra::DynamicVector<double>
{
    return linear_algebra::DynamicVector<double>(mCriterionInterface->value(aMeshDesignVariables));
}

auto SharedLibraryVectorCriterion::jacobianTimesVector(
    const mesh::MeshDesignVariables& aMeshDesignVariables,
    const linear_algebra::DynamicVector<double>& aDirectionVector) const -> linear_algebra::DynamicVector<double>
{
    return linear_algebra::DynamicVector<double>(
        mCriterionInterface->jacobianTimesVector(aMeshDesignVariables, aDirectionVector.stdVector()));
}

auto SharedLibraryVectorCriterion::adjointJacobianTimesVector(
    const mesh::MeshDesignVariables& aMeshDesignVariables,
    const linear_algebra::DynamicVector<double>& aDualVector) const -> linear_algebra::DynamicVector<double>
{
    return linear_algebra::DynamicVector<double>(
        mCriterionInterface->adjointJacobianTimesVector(aMeshDesignVariables, aDualVector.stdVector()));
}

namespace
{
auto make_jacobian_multiplier(const SharedLibraryVectorCriterion& aSharedLibCriterion,
                              const mesh::MeshDesignVariables& aMeshDesignVariables)
    -> linear_algebra::JacobianMultiplier
{
    return linear_algebra::JacobianMultiplier{[aSharedLibCriterion, aMeshDesignVariables](const auto aDirectionVector) {
        return aSharedLibCriterion.jacobianTimesVector(aMeshDesignVariables, aDirectionVector);
    }};
}
auto make_adjoint_jacobian_multiplier(const SharedLibraryVectorCriterion& aSharedLibCriterion,
                                      const mesh::MeshDesignVariables& aMeshDesignVariables)
    -> linear_algebra::JacobianMultiplier
{
    return linear_algebra::JacobianMultiplier{[aSharedLibCriterion, aMeshDesignVariables](const auto aDualVector) {
        return aSharedLibCriterion.adjointJacobianTimesVector(aMeshDesignVariables, aDualVector);
    }};
}

}  // namespace

auto make_shared_library_jacobian_function(const SharedLibraryVectorCriterion& aSharedLibCriterion)
    -> core::Function<linear_algebra::DynamicVector<double>,
                      linear_algebra::JacobianMultiplier,
                      const mesh::MeshDesignVariables&>
{
    return core::make_function([aSharedLibCriterion](const mesh::MeshDesignVariables& aMeshDesignVariables)
                               { return aSharedLibCriterion.value(aMeshDesignVariables); },
                               [aSharedLibCriterion](const mesh::MeshDesignVariables& aMeshDesignVariables)
                               { return make_jacobian_multiplier(aSharedLibCriterion, aMeshDesignVariables); });
}

auto make_shared_library_adjoint_jacobian_function(const SharedLibraryVectorCriterion& aSharedLibCriterion)
    -> core::Function<linear_algebra::DynamicVector<double>,
                      linear_algebra::JacobianMultiplier,
                      const mesh::MeshDesignVariables&>
{
    return core::make_function([aSharedLibCriterion](const mesh::MeshDesignVariables& aMeshDesignVariables)
                               { return aSharedLibCriterion.value(aMeshDesignVariables); },
                               [aSharedLibCriterion](const mesh::MeshDesignVariables& aMeshDesignVariables)
                               { return make_adjoint_jacobian_multiplier(aSharedLibCriterion, aMeshDesignVariables); });
}

}  // namespace plato::criteria::extension
