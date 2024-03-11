#ifndef PLATO_INTEGRATION_TESTS_TEST_MASS_OBJECTIVE_MASSOBJECTIVEINTERFACE
#define PLATO_INTEGRATION_TESTS_TEST_MASS_OBJECTIVE_MASSOBJECTIVEINTERFACE

#include "plato/criteria/library/CriterionInterface.hpp"

namespace plato::integration_tests::test_mass_objective
{
class MassObjectiveInterface : public criteria::library::CriterionInterface
{
   public:
    ///@brief Construct a new Mass Objective Interface object
    MassObjectiveInterface();

    ///@brief required by the CriterionInterface, return the value of the criterion evaluated at the controls specified
    /// by the MeshProxy
    double value(const core::MeshProxy& aMeshProxy) const override;

    ///@brief required by the CriterionInterface, return the gradient of the criterion evaluated at the controls
    /// specified by the MeshProxy
    std::vector<double> gradient(const core::MeshProxy& aMeshProxy) const override;
};
}  // namespace plato::integration_tests::test_mass_objective

#endif
