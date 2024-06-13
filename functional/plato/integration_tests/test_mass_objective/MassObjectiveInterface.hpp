#ifndef PLATO_INTEGRATION_TESTS_TEST_MASS_OBJECTIVE_MASSOBJECTIVEINTERFACE
#define PLATO_INTEGRATION_TESTS_TEST_MASS_OBJECTIVE_MASSOBJECTIVEINTERFACE

#include "plato/criteria/library/CriterionInterface.hpp"

namespace plato::integration_tests::test_mass_objective
{
/// @brief Serial version of a test criterion that computes the mass of a mesh.
///
/// The main difference between this and the parallel interface (ParallelMassObjectiveInterface) is that this
/// version's constructor has no arguments. This implements plato_create_criterion
/// rather than plato_create_parallel_criterion, which indicates to plato that a serial interface can be used.
/// @note This interface is only used when the `number_of_processors` field is equal to one. Otherwise,
/// ParallelMassObjectiveInterface is used.
/// @sa ParallelMassObjectiveInterface
class MassObjectiveInterface : public criteria::library::CriterionInterface
{
   public:
    ///@brief required by the CriterionInterface, return the value of the criterion evaluated at the controls specified
    /// by the MeshProxy
    double value(const core::MeshProxy& aMeshProxy) const override;

    ///@brief required by the CriterionInterface, return the gradient of the criterion evaluated at the controls
    /// specified by the MeshProxy
    std::vector<double> gradient(const core::MeshProxy& aMeshProxy) const override;
};
}  // namespace plato::integration_tests::test_mass_objective

#endif
