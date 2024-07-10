#include "plato/filter/test_utilities/SharedLibIdentityFilter.hpp"

#include "plato/mesh/MeshProxy.hpp"

namespace plato::filter::test_utilities
{
mesh::MeshProxy SharedLibIdentityFilter::filter(const mesh::MeshProxy& aMeshProxy) const { return aMeshProxy; }

linear_algebra::DynamicVector<double> SharedLibIdentityFilter::jacobianTimesVector(
    const mesh::MeshProxy&, const linear_algebra::DynamicVector<double>& aV) const
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
