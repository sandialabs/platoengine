#include "plato/filter/test_utilities/SharedLibIdentityFilter.hpp"

#include "plato/design_variables/MeshDesignVariables.hpp"

namespace plato::filter::test_utilities
{
design_variables::MeshDesignVariables SharedLibIdentityFilter::filter(
    const design_variables::MeshDesignVariables& aMeshDesignVariables) const
{
    return aMeshDesignVariables;
}

linear_algebra::DynamicVector<double> SharedLibIdentityFilter::jacobianTimesVector(
    const design_variables::MeshDesignVariables&, const linear_algebra::DynamicVector<double>& aV) const
{
    return aV;
}

}  // namespace plato::filter::test_utilities

namespace plato
{
std::unique_ptr<filter::library::FilterInterface> plato_create_filter(const filter::library::FilterParameters&)
{
    return std::make_unique<filter::test_utilities::SharedLibIdentityFilter>();
}
}  // namespace plato
