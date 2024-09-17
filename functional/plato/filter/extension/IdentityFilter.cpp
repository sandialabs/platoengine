#include "plato/filter/extension/IdentityFilter.hpp"

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"
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

analysis::AnalysisDomainMesh IdentityFilter::filter(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const
{
    return aAnalysisDomainMesh;
}

linear_algebra::DynamicVector<double> IdentityFilter::jacobianTimesVector(
    const analysis::AnalysisDomainMesh& aAnalysisDomainMesh, const linear_algebra::DynamicVector<double>& aV) const
{
    const auto tVectorDimension = static_cast<std::size_t>(aV.size());
    const std::size_t tDensityDimension = analysis::AnalysisDomainMeshSequentialView{aAnalysisDomainMesh}.size();
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
    -> core::Function<analysis::AnalysisDomainMesh, library::FilterJacobian, const analysis::AnalysisDomainMesh&>
{
    return core::make_function(
        [](const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
        { return IdentityFilter{}.filter(aAnalysisDomainMesh); },
        [](const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) {
            return library::FilterJacobian{std::make_unique<IdentityFilter>(), aAnalysisDomainMesh};
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
