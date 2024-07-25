#include "plato/filter/extension/IdentityFilter.hpp"

#include "plato/core/ValidationRegistration.hpp"
#include "plato/filter/library/FilterJacobian.hpp"
#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/mesh/MeshDesignVariables.hpp"
#include "plato/mesh/MeshDesignVariablesViews.hpp"
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

mesh::MeshDesignVariables IdentityFilter::filter(const mesh::MeshDesignVariables& aMeshDesignVariables) const
{
    return aMeshDesignVariables;
}

linear_algebra::DynamicVector<double> IdentityFilter::jacobianTimesVector(
    const mesh::MeshDesignVariables& aMeshDesignVariables, const linear_algebra::DynamicVector<double>& aV) const
{
    const auto tVectorDimension = static_cast<std::size_t>(aV.size());
    const std::size_t tDensityDimension = mesh::MeshDesignVariablesDensitiesView{aMeshDesignVariables}.size();
    if (tVectorDimension != tDensityDimension)
    {
        throw utilities::Exception{
            "IdentityFilter jacobian multiplication: Dimensions of vector and nodal density field don't match. Vector "
            "dimension: " +
            std::to_string(tVectorDimension) + ", density dimension: " + std::to_string(tDensityDimension)};
    }
    return aV;
}

auto make_identity_filter_function()
    -> core::Function<mesh::MeshDesignVariables, library::FilterJacobian, const mesh::MeshDesignVariables&>
{
    return core::make_function(
        [](const mesh::MeshDesignVariables& aMeshDesignVariables)
        { return IdentityFilter{}.filter(aMeshDesignVariables); },
        [](const mesh::MeshDesignVariables& aMeshDesignVariables) {
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
