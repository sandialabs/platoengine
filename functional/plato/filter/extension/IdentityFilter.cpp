#include "plato/filter/extension/IdentityFilter.hpp"

#include "plato/core/ValidationRegistration.hpp"
#include "plato/design_variables/MeshDesignVariables.hpp"
#include "plato/design_variables/MeshDesignVariablesSequentialView.hpp"
#include "plato/filter/library/FilterJacobian.hpp"
#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/utilities/Exception.hpp"

namespace plato::filter::extension
{
namespace
{
[[maybe_unused]] static auto kIdentityFilterRegistration =
    library::FilterRegistration{input_parser::block_name<input_parser::identity_filter>(),
                                [](const library::ValidatedFilterInput&) { return make_identity_filter_function(); }};

[[maybe_unused]] static auto kIdentityFilterValidationRegistration =
    core::ValidationRegistration<input_parser::identity_filter>{[](const input_parser::identity_filter& aInput)
                                                                { return validate_identity_filter(aInput); }};
}  // namespace

design_variables::MeshDesignVariables IdentityFilter::filter(
    const design_variables::MeshDesignVariables& aMeshDesignVariables) const
{
    return aMeshDesignVariables;
}

linear_algebra::DynamicVector<double> IdentityFilter::jacobianTimesVector(
    const design_variables::MeshDesignVariables& aMeshDesignVariables,
    const linear_algebra::DynamicVector<double>& aV) const
{
    const auto tVectorDimension = static_cast<std::size_t>(aV.size());
    const std::size_t tDensityDimension =
        design_variables::MeshDesignVariablesSequentialView{aMeshDesignVariables}.size();
    if (tVectorDimension != tDensityDimension)
    {
        throw utilities::Exception{
            "IdentityFilter jacobian multiplication: Dimensions of vector and nodal density field don't match. Vector "
            "dimension: " +
            std::to_string(tVectorDimension) + ", density dimension: " + std::to_string(tDensityDimension)};
    }
    return aV;
}

auto make_identity_filter_function() -> core::Function<design_variables::MeshDesignVariables,
                                                       library::FilterJacobian,
                                                       const design_variables::MeshDesignVariables&>
{
    return core::make_function(
        [](const design_variables::MeshDesignVariables& aMeshDesignVariables)
        { return IdentityFilter{}.filter(aMeshDesignVariables); },
        [](const design_variables::MeshDesignVariables& aMeshDesignVariables) {
            return library::FilterJacobian{std::make_unique<IdentityFilter>(), aMeshDesignVariables};
        });
}

[[nodiscard]] std::optional<std::string> validate_identity_filter(const input_parser::identity_filter& aInput)
{
    if (aInput.filter_radius.has_value())
    {
        return input_parser::block_name<input_parser::identity_filter>() +
               R"( identity filter cannot have "filter_radius" defined.)";
    }
    return std::nullopt;
}

}  // namespace plato::filter::extension
