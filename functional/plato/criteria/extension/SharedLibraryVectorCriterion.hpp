#ifndef PLATO_CRITERIA_EXTENSION_SHAREDLIBRARYVECTORCRITERION
#define PLATO_CRITERIA_EXTENSION_SHAREDLIBRARYVECTORCRITERION

#include <boost/mpi/communicator.hpp>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "plato/core/Function.hpp"
#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/criteria/library/VectorCriterionInterface.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianMultiplier.hpp"
#include "plato/mesh/MeshDesignVariables.hpp"

namespace plato::services
{
struct AppConfigurationWithDirectory;
}

namespace plato::criteria::extension
{
/// @brief for a vector criterion that is loaded from a shared library.
///
/// A shared library path is given on construction from which to load
/// a VectorCriterionInterface object.
class SharedLibraryVectorCriterion
{
   public:
    SharedLibraryVectorCriterion(const services::AppConfigurationWithDirectory& aAppConfiguration,
                                 const services::CriterionConfiguration& aCriterionConfiguration,
                                 const std::vector<std::string>& aFileNames);
    SharedLibraryVectorCriterion(const services::AppConfigurationWithDirectory& aAppConfiguration,
                                 const services::CriterionConfiguration& aCriterionConfiguration,
                                 const std::vector<std::string>& aFileNames,
                                 const boost::mpi::communicator& aComm);

    [[nodiscard]] auto value(const mesh::MeshDesignVariables& aMesh) const -> linear_algebra::DynamicVector<double>;

    [[nodiscard]] auto jacobianTimesVector(const mesh::MeshDesignVariables& aMesh,
                                           const linear_algebra::DynamicVector<double>& aDirectionVector) const
        -> linear_algebra::DynamicVector<double>;
    [[nodiscard]] auto adjointJacobianTimesVector(const mesh::MeshDesignVariables& aMesh,
                                                  const linear_algebra::DynamicVector<double>& aDualVector) const
        -> linear_algebra::DynamicVector<double>;

   private:
    std::shared_ptr<library::VectorCriterionInterface> mCriterionInterface;
    boost::mpi::communicator mComm{MPI_COMM_NULL, boost::mpi::comm_attach};
};

[[nodiscard]] auto make_shared_library_jacobian_function(const SharedLibraryVectorCriterion& aSharedLibCriterion)
    -> core::Function<linear_algebra::DynamicVector<double>,
                      linear_algebra::JacobianMultiplier,
                      const mesh::MeshDesignVariables&>;

[[nodiscard]] auto make_shared_library_adjoint_jacobian_function(
    const SharedLibraryVectorCriterion& aSharedLibCriterion) -> core::Function<linear_algebra::DynamicVector<double>,
                                                                               linear_algebra::JacobianMultiplier,
                                                                               const mesh::MeshDesignVariables&>;

}  // namespace plato::criteria::extension

#endif
