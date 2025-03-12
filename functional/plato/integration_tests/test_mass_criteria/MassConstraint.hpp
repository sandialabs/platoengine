#ifndef PLATO_INTEGRATIONTESTS_TESTVECTORCONSTRAINT_MASSCONSTRAINT
#define PLATO_INTEGRATIONTESTS_TESTVECTORCONSTRAINT_MASSCONSTRAINT

#include <string_view>
#include <vector>

namespace plato::integration_tests::test_mass_criteria
{
///@brief A simple class that can be used to impose an additional constraint on the densities
class MassConstraint
{
   public:
    ///@brief Construct a new DensityConstraint object
    explicit MassConstraint(const double aDensity);

    ///@brief return the vector of the individual masses of the elements in a mesh @a aMeshFileName
    [[nodiscard]] std::vector<double> masses(std::string_view aMeshFileName) const;

   private:
    double mDensity = 1;
};

}  // namespace plato::integration_tests::test_mass_criteria

#endif
