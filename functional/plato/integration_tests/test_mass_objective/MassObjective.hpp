#ifndef PLATO_INTEGRATION_TESTS_TEST_MASS_OBJECTIVE_MASSOBJECTIVE
#define PLATO_INTEGRATION_TESTS_TEST_MASS_OBJECTIVE_MASSOBJECTIVE

#include <string>

namespace plato::integration_tests::test_mass_objective
{
class MassObjective
{
   public:
    ///@brief Construct a new Mass Objective object
    explicit MassObjective(const double aDensity);

    ///@brief return the total mass of the mesh
    [[nodiscard]] double mass(std::string_view aMeshFileName) const;

   private:
    double mDensity = 1.0;
};

}  // namespace plato::integration_tests::test_mass_objective

#endif
