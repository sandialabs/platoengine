#ifndef PLATO_INTEGRATION_TESTS_TEST_MASS_OBJECTIVE_PARALLELMASSOBJECTIVEINTERFACE
#define PLATO_INTEGRATION_TESTS_TEST_MASS_OBJECTIVE_PARALLELMASSOBJECTIVEINTERFACE

#include <mpi.h>

#include <boost/mpi/communicator.hpp>

#include "plato/criteria/library/CriterionInterface.hpp"

namespace plato::integration_tests::test_mass_objective
{
/// @brief Parallel version of a test criterion that computes the mass of a mesh.
///
/// The main difference between this and the serial interface (MassObjectiveInterface) is that this
/// has a constructor with an MPI_Comm argument. Also, this implements plato_create_parallel_criterion
/// rather than plato_create_criterion, which indicates to plato that a parallel interface can be used.
/// @note This interface is only used when the `number_of_processors` field is greater than one. Otherwise,
/// MassObjectiveInterface is used.
/// @sa MassObjectiveInterface
class ParallelMassObjectiveInterface : public criteria::library::CriterionInterface
{
   public:
    explicit ParallelMassObjectiveInterface(MPI_Comm aComm);

    ///@brief required by the CriterionInterface, return the value of the criterion evaluated at the controls specified
    /// by the MeshProxy
    [[nodiscard]] double value(const core::MeshProxy& aMeshProxy) const override;

    ///@brief required by the CriterionInterface, return the gradient of the criterion evaluated at the controls
    /// specified by the MeshProxy
    [[nodiscard]] std::vector<double> gradient(const core::MeshProxy& aMeshProxy) const override;

   private:
    boost::mpi::communicator mComm;
};
}  // namespace plato::integration_tests::test_mass_objective

#endif
