#ifndef PLATO_INTEGRATION_TESTS_TEST_MASS_OBJECTIVE_MASSOBJECTIVE
#define PLATO_INTEGRATION_TESTS_TEST_MASS_OBJECTIVE_MASSOBJECTIVE

#include <string>

namespace plato::integration_tests::test_mass_criteria
{
class MassObjective
{
   public:
    ///@brief Construct a new Mass Objective object
    explicit MassObjective(double aDensity);

    ///@brief return the total mass of the mesh
    [[nodiscard]] auto mass(std::string_view aMeshFileName) const -> double;

   private:
    double mDensity = 1.0;
};

}  // namespace plato::integration_tests::test_mass_criteria

#endif
