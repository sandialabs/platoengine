#ifndef PLATO_INTEGRATION_TESTS_TEST_MASS_OBJECTIVE_MASSOBJECTIVE
#define PLATO_INTEGRATION_TESTS_TEST_MASS_OBJECTIVE_MASSOBJECTIVE

#include <string>

namespace plato::integration_tests::test_mass_objective
{
class MassObjective
{
   public:
    ///@brief Construct a new Mass Objective object
    MassObjective(const double aDensity, const double aTarget);

    ///@brief return the total mass of the mesh
    [[nodiscard]] double mass(std::string_view aMeshFileName) const;

    ///@brief return the number of nodes in the mesh
    [[nodiscard]] unsigned int numMeshNodes(std::string_view aMeshFileName) const;

   private:
    double mDensity = 1.0;
    double mTarget = 0.0;
};

}  // namespace plato::integration_tests::test_mass_objective

#endif
