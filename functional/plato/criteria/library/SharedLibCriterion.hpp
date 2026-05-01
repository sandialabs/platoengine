#ifndef PLATO_CRITERIA_LIBRARY_SHAREDLIBCRITERION
#define PLATO_CRITERIA_LIBRARY_SHAREDLIBCRITERION

#include <boost/mpi/communicator.hpp>
#include <memory>
#include <string>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/criteria/library/CriterionInterface.hpp"
#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/services/SharedLibraryObject.hpp"

namespace plato::services
{
struct AppConfigurationWithDirectory;
}

namespace plato::criteria::library
{
using CriterionSharedLibraryObject = services::SharedLibraryObject<std::unique_ptr<library::CriterionInterface>>;

/// @brief for a criterion that is loaded from a shared library.
///
/// A shared library path is given on construction from which to load
/// a CriterionInterface object.
class SharedLibCriterion
{
   public:
    SharedLibCriterion(const services::AppConfigurationWithDirectory& aAppConfiguration,
                       const services::CriterionConfiguration& aCriterionConfiguration,
                       const library::CriterionInput& aCriterionInput);
    SharedLibCriterion(const services::AppConfigurationWithDirectory& aAppConfiguration,
                       const services::CriterionConfiguration& aCriterionConfiguration,
                       const library::CriterionInput& aCriterionInput,
                       const boost::mpi::communicator& aComm);

    [[nodiscard]] double f(const analysis::AnalysisDomainMesh& aMesh) const;

    [[nodiscard]] linear_algebra::DynamicVector<double> df(const analysis::AnalysisDomainMesh& aMesh) const;

   private:
    std::shared_ptr<CriterionSharedLibraryObject> mCriterionInterface;
    boost::mpi::communicator mComm{MPI_COMM_NULL, boost::mpi::comm_attach};
    components::ComponentType mComponentType;
    std::string mName;
};

[[nodiscard]] auto make_shared_lib_function(const SharedLibCriterion& aSharedLibCriterion)
    -> library::CriterionFunction;

}  // namespace plato::criteria::library

#endif
