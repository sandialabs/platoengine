#ifndef PLATO_INTEGRATION_TESTS_TESTVECTORCONSTRAINT_MASSCONSTRAINTVECTORINTERFACE
#define PLATO_INTEGRATION_TESTS_TESTVECTORCONSTRAINT_MASSCONSTRAINTVECTORINTERFACE

#include "plato/criteria/library/VectorCriterionInterface.hpp"

namespace plato::integration_tests::test_mass_criteria
{
constexpr double kDensity = 2.5;

/// @brief Serial version of a test constraint that bounds a 2D solution within a number of constraints
class MassConstraintInterface : public criteria::library::VectorCriterionInterface
{
   public:
    ///@brief Required by the VectorCriterionInterface, returns the value of the constraint evaluated at the controls
    /// specified by the MeshDesignVariables
    std::vector<double> value(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const override;

    ///@brief Required by the VectorCriterionInterface, returns the product of a row vector @a aDirectionVector with the
    /// Jacobian evaluated at the controls specified by @a aAnalysisDomainMesh.
    std::vector<double> rowVectorTimesJacobian(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                               const std::vector<double>& aDirectionVector) const override;

    ///@brief Required by the VectorCriterionInterface, return the adjoint jacobian times a dual vector of the
    /// constraint evaluated at the controls specified by the MeshDesignVariables and the supplied direction vector
    std::vector<double> rowVectorTimesAdjointJacobian(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                                      const std::vector<double>& aDualVector) const override;
};
}  // namespace plato::integration_tests::test_mass_criteria

extern "C" auto plato_create_vector_criterion(const std::vector<std::string>&)
    -> std::unique_ptr<::plato::criteria::library::VectorCriterionInterface>;

extern "C" auto plato_create_vector_test_mass_criterion(const std::vector<std::string>&)
    -> std::unique_ptr<::plato::criteria::library::VectorCriterionInterface>;

#endif
