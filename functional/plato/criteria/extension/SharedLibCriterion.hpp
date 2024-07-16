#ifndef PLATO_CRITERIA_EXTENSION_SHAREDLIBCRITERION
#define PLATO_CRITERIA_EXTENSION_SHAREDLIBCRITERION

#include <boost/mpi/communicator.hpp>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "plato/core/Function.hpp"
#include "plato/core/MeshProxy.hpp"
#include "plato/criteria/library/CriterionInterface.hpp"
#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"

namespace plato::services
{
struct AppConfigurationWithDirectory;
}

namespace plato::criteria::extension
{
/// @brief for a criterion that is loaded from a shared library.
///
/// A shared library path is given on construction from which to load
/// a CriterionInterface object.
class SharedLibCriterion
{
   public:
    SharedLibCriterion(const services::AppConfigurationWithDirectory& aAppConfiguration,
                       const services::CriterionConfiguration& aCriterionConfiguration,
                       const std::vector<std::string>& aFileNames);
    SharedLibCriterion(const services::AppConfigurationWithDirectory& aAppConfiguration,
                       const services::CriterionConfiguration& aCriterionConfiguration,
                       const std::vector<std::string>& aFileNames,
                       const boost::mpi::communicator& aComm);

    [[nodiscard]] double f(const core::MeshProxy& aMesh) const;

    [[nodiscard]] linear_algebra::DynamicVector<double> df(const core::MeshProxy& aMesh) const;

   private:
    std::shared_ptr<library::CriterionInterface> mCriterionInterface;
    boost::mpi::communicator mComm{MPI_COMM_NULL, boost::mpi::comm_attach};
};

[[nodiscard]] auto make_shared_lib_function(const SharedLibCriterion& aSharedLibCriterion)
    -> core::Function<double, linear_algebra::DynamicVector<double>, const core::MeshProxy&>;

}  // namespace plato::criteria::extension

#endif
