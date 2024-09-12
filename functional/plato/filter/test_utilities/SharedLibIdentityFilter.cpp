#include "plato/filter/test_utilities/SharedLibIdentityFilter.hpp"

#include "plato/mesh/MeshDesignVariables.hpp"

namespace plato::filter::test_utilities
{
mesh::MeshDesignVariables SharedLibIdentityFilter::filter(const mesh::MeshDesignVariables& aMeshDesignVariables) const
{
    return aMeshDesignVariables;
}

linear_algebra::DynamicVector<double> SharedLibIdentityFilter::jacobianTimesVector(
    const mesh::MeshDesignVariables&, const linear_algebra::DynamicVector<double>& aV) const
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
