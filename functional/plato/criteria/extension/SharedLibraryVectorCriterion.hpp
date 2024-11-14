#ifndef PLATO_CRITERIA_EXTENSION_SHAREDLIBRARYVECTORCRITERION
#define PLATO_CRITERIA_EXTENSION_SHAREDLIBRARYVECTORCRITERION

#include <boost/mpi/communicator.hpp>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/core/Function.hpp"
#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/criteria/library/VectorCriterionInterface.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianMultiplier.hpp"

namespace plato::services
{
struct AppConfigurationWithDirectory;
}

namespace plato::criteria::extension
{
/// @brief for a vector criterion that is loaded from a shared library.
///
/// A shared library path is given on construction from which to load a VectorCriterionInterface object.
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

    /// @brief Computes the value of the criterion evaluated at the argument @a aAnalysisDomainMesh.
    [[nodiscard]] auto value(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const
        -> linear_algebra::DynamicVector<double>;

    /// @brief Computes the left multiplication of a row vector @a aDirectionVector with the Jacobian matrix, evaluated
    /// at the argument @a aAnalysisDomainMesh.
    [[nodiscard]] auto rowVectorTimesJacobian(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                              const linear_algebra::DynamicVector<double>& aDirectionVector) const
        -> linear_algebra::DynamicVector<double>;

    /// @brief Computes the left multiplication of a row vector @a aDualVector with the adjoint of the Jacobian matrix,
    /// evaluated at the argument @a aAnalysisDomainMesh..
    [[nodiscard]] auto rowVectorTimesAdjointJacobian(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                                     const linear_algebra::DynamicVector<double>& aDualVector) const
        -> linear_algebra::DynamicVector<double>;

   private:
    std::shared_ptr<library::VectorCriterionInterface> mCriterionInterface;
    boost::mpi::communicator mComm{MPI_COMM_NULL, boost::mpi::comm_attach};
};

using SharedLibraryVectorCriterionFunction =
    core::Function<const analysis::AnalysisDomainMesh&,
                   core::FunctionInfo<linear_algebra::DynamicVector<double>, core::evaluation::kFunction>,
                   core::FunctionInfo<linear_algebra::JacobianMultiplier, core::evaluation::kFirstDerivative>,
                   core::FunctionInfo<linear_algebra::AdjointJacobianMultiplier,
                                      core::evaluation::kFirstDerivative,
                                      core::MatrixOrdering::kAdjoint> >;

[[nodiscard]] auto make_shared_library_jacobian_function(const SharedLibraryVectorCriterion& aSharedLibCriterion)
    -> SharedLibraryVectorCriterionFunction;

}  // namespace plato::criteria::extension

#endif
