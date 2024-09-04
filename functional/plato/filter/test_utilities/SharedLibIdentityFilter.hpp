#ifndef PLATO_FILTER_TESTUTILITIES_SHAREDLIBIDENTITYFILTER
#define PLATO_FILTER_TESTUTILITIES_SHAREDLIBIDENTITYFILTER

#include "plato/filter/library/FilterInterface.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"

namespace plato::design_variables
{
struct MeshDesignVariables;
}

namespace plato::filter::test_utilities
{
/// @brief The purpose of this filter is to test the shared library interface.
///
/// It is an identity filter, meaning that the density field will be unchanged on
/// calls to `filter` and its Jacobian is the identity matrix.
class SharedLibIdentityFilter : public library::FilterInterface
{
   public:
    [[nodiscard]] design_variables::MeshDesignVariables filter(
        const design_variables::MeshDesignVariables& aMeshDesignVariables) const override;

    [[nodiscard]] linear_algebra::DynamicVector<double> jacobianTimesVector(
        const design_variables::MeshDesignVariables& aMeshDesignVariables,
        const linear_algebra::DynamicVector<double>& aV) const override;
};
}  // namespace plato::filter::test_utilities

#endif
