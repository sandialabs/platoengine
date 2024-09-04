#ifndef PLATO_FILTER_EXTENSION_IDENTITYFILTER
#define PLATO_FILTER_EXTENSION_IDENTITYFILTER

#include "plato/core/Function.hpp"
#include "plato/filter/library/FilterInterface.hpp"

namespace plato::input_parser
{
struct identity_filter;
}
namespace plato::design_variables
{
struct MeshDesignVariables;
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
    [[nodiscard]] design_variables::MeshDesignVariables filter(
        const design_variables::MeshDesignVariables& aMeshDesignVariables) const override;

    [[nodiscard]] linear_algebra::DynamicVector<double> jacobianTimesVector(
        const design_variables::MeshDesignVariables& aMeshDesignVariables,
        const linear_algebra::DynamicVector<double>& aV) const override;
};

[[nodiscard]] auto make_identity_filter_function() -> core::Function<design_variables::MeshDesignVariables,
                                                                     library::FilterJacobian,
                                                                     const design_variables::MeshDesignVariables&>;

[[nodiscard]] std::optional<std::string> validate_identity_filter(const input_parser::identity_filter& aInput);

}  // namespace plato::filter::extension

#endif
