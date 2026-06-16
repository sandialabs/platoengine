#include "plato/filter/extension/IdentityFilter.hpp"

#include <cstddef>
#include <format>
#include <memory>
#include <optional>
#include <string>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"
#include "plato/filter/library/FilterJacobian.hpp"
#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/input_parser/ComponentParserRegistration.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_validation/ValidationRegistration.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/utilities/Exception.hpp"

namespace plato::filter::extension
{
namespace
{
[[maybe_unused]] const auto kIdentityFilterParserRegistration =
    input_parser::ComponentParserRegistration<input_parser::identity_filter>{};

[[maybe_unused]] const auto kIdentityFilterRegistration =
    library::FilterRegistration{input_parser::block_name<input_parser::identity_filter>(),
                                [](const library::ValidatedFilterInput&) { return make_identity_filter_function(); }};

[[maybe_unused]] const auto kIdentityFilterValidationRegistration =
    input_validation::InputBlockValidationRegistration<>{[](const input_parser::identity_filter& aInput)
                                                         { return validate_identity_filter(aInput); }};
}  // namespace

auto IdentityFilter::filter(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const
    -> analysis::AnalysisDomainMesh
{
    return aAnalysisDomainMesh;
}

auto IdentityFilter::rowVectorTimesJacobian(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                            const linear_algebra::DynamicVector<double>& aRowVector) const
    -> linear_algebra::DynamicVector<double>
{
    const auto tVectorDimension = static_cast<std::size_t>(aRowVector.size());
    const std::size_t tDensityDimension = analysis::AnalysisDomainMeshSequentialView{aAnalysisDomainMesh}.size();
    if (tVectorDimension != tDensityDimension)
    {
        throw utilities::Exception{
            "IdentityFilter jacobian multiplication: Dimensions of vector and nodal density field don't match. Vector "
            "dimension: " +
            std::to_string(tVectorDimension) + ", density dimension: " + std::to_string(tDensityDimension)};
    }
    return aRowVector;
}

auto IdentityFilter::rowVectorTimesAdjointJacobian(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                                   const linear_algebra::DynamicVector<double>& aRowVector) const
    -> linear_algebra::DynamicVector<double>
{
    return rowVectorTimesJacobian(aAnalysisDomainMesh, aRowVector);
}

auto make_identity_filter_function() -> library::FilterFunction
{
    return library::FilterFunction{
        [](const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
        { return IdentityFilter{}.filter(aAnalysisDomainMesh); },
        [](const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
        { return library::make_filter_jacobian(std::make_unique<IdentityFilter>(), aAnalysisDomainMesh); },
        [](const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
        { return library::make_filter_adjoint_jacobian(std::make_unique<IdentityFilter>(), aAnalysisDomainMesh); }};
}

auto validate_identity_filter(const input_parser::identity_filter& aInput) -> std::optional<std::string>
{
    if (aInput.filter_radius.has_value())
    {
        return std::format(R"({} identity filter cannot have "filter_radius" defined.)",
                           input_parser::block_name<input_parser::identity_filter>());
    }
    return std::nullopt;
}

}  // namespace plato::filter::extension
