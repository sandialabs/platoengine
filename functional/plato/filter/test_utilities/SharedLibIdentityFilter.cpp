#include "plato/filter/test_utilities/SharedLibIdentityFilter.hpp"

#include "plato/core/MeshProxy.hpp"

namespace plato::filter::test_utilities
{
core::MeshProxy SharedLibIdentityFilter::filter(const core::MeshProxy& aMeshProxy) const { return aMeshProxy; }

linear_algebra::DynamicVector<double> SharedLibIdentityFilter::jacobianTimesVector(
    const core::MeshProxy&, const linear_algebra::DynamicVector<double>& aV) const
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
