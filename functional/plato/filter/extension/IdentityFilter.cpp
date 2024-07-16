#include "plato/filter/extension/IdentityFilter.hpp"

#include "plato/core/MeshProxy.hpp"
#include "plato/core/ValidationRegistration.hpp"
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

core::MeshProxy IdentityFilter::filter(const core::MeshProxy& aMeshProxy) const { return aMeshProxy; }

linear_algebra::DynamicVector<double> IdentityFilter::jacobianTimesVector(
    const core::MeshProxy& aMeshProxy, const linear_algebra::DynamicVector<double>& aV) const
{
    const auto tVectorDimension = static_cast<std::size_t>(aV.size());
    const std::size_t tDensityDimension = aMeshProxy.mNodalDensities.size();
    if (tVectorDimension != tDensityDimension)
    {
        throw utilities::Exception{
            "IdentityFilter jacobian multiplication: Dimensions of vector and nodal density field don't match. Vector "
            "dimension: " +
            std::to_string(tVectorDimension) + ", density dimension: " + std::to_string(tDensityDimension)};
    }
    return aV;
}

auto make_identity_filter_function() -> core::Function<core::MeshProxy, library::FilterJacobian, const core::MeshProxy&>
{
    return core::make_function([](const core::MeshProxy& aMeshProxy) { return IdentityFilter{}.filter(aMeshProxy); },
                               [](const core::MeshProxy& aMeshProxy) {
                                   return library::FilterJacobian{std::make_unique<IdentityFilter>(), aMeshProxy};
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
