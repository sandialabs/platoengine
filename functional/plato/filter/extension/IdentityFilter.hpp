#ifndef PLATO_FILTER_EXTENSION_IDENTITYFILTER
#define PLATO_FILTER_EXTENSION_IDENTITYFILTER

#include "plato/core/Function.hpp"
#include "plato/filter/library/FilterInterface.hpp"

namespace plato::input_parser
{
struct density_topology;
}
namespace plato::core
{
struct MeshProxy;
}
namespace plato::filter::library
{
struct FilterJacobian;
}

namespace plato::filter::extension
{
/// @brief A Filter that does not alter the density field, mostly used for testing.
///
/// Calls to the filter member function return the same density field,
/// and the Jacobian is the identity matrix.
class IdentityFilter : public library::FilterInterface
{
   public:
    [[nodiscard]] core::MeshProxy filter(const core::MeshProxy& aMeshProxy) const override;

    [[nodiscard]] linear_algebra::DynamicVector<double> jacobianTimesVector(
        const core::MeshProxy& aMeshProxy, const linear_algebra::DynamicVector<double>& aV) const override;
};

[[nodiscard]] auto make_identity_filter_function()
    -> core::Function<core::MeshProxy, library::FilterJacobian, const core::MeshProxy&>;

[[nodiscard]] std::optional<std::string> validate_identity_filter(const input_parser::density_topology& aInput);

}  // namespace plato::filter::extension

#endif
