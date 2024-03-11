#include "plato/filter/extension/HelmholtzFilter.hpp"

#include <filesystem>

#include "plato/core/ValidationRegistration.hpp"
#include "plato/core/ValidationUtilities.hpp"
#include "plato/filter/library/FilterInterface.hpp"
#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/input_parser/InputEnumTypes.hpp"

namespace plato::filter::extension
{
namespace
{
const auto kHelmholtzFilterLibName = std::filesystem::path{"libAnalyzeFunctionalInterface.so"};

[[maybe_unused]] static auto kHelmholtzFilterRegistration = library::FilterRegistration{
    input_parser::kFilterTypesTable.toString(input_parser::FilterTypes::kHelmholtz).value(),
    [](const input_parser::density_topology& aInput)
    { return library::make_filter_function_from_interface(library::load_filter(aInput, kHelmholtzFilterLibName)); }};

[[maybe_unused]] static auto kHelmholtzFilterValidationRegistration =
    core::ValidationRegistration<input_parser::density_topology>{
        [](const input_parser::density_topology& aInput) { return validate_helmholtz_filter_radius(aInput); },
        [](const input_parser::density_topology& aInput)
        { return validate_helmholtz_filter_boundary_sticking_penalty(aInput); }};
}  // namespace

std::optional<std::string> validate_helmholtz_filter_radius(const input_parser::density_topology& aInput)
{
    namespace pfu = plato::utilities;
    if (aInput.filter_type && aInput.filter_type.value() == input_parser::FilterTypes::kHelmholtz)
    {
        return core::error_message_for_parameter_out_of_bounds(
            input_parser::block_name<input_parser::density_topology>(), aInput.filter_radius, "filter_radius",
            pfu::lower_bounded(pfu::Exclusive{0.0}));
    }
    else
    {
        return std::nullopt;
    }
}

[[nodiscard]] std::optional<std::string> validate_helmholtz_filter_boundary_sticking_penalty(
    const input_parser::density_topology& aInput)
{
    namespace pfu = plato::utilities;
    if (aInput.filter_type && aInput.filter_type.value() == input_parser::FilterTypes::kHelmholtz &&
        aInput.boundary_sticking_penalty)
    {
        // boundary_sticking_penalty is optional, so only validate bounds if it exists
        return core::error_message_for_parameter_out_of_bounds(
            input_parser::block_name<input_parser::density_topology>(), aInput.boundary_sticking_penalty,
            "boundary_sticking_penalty", pfu::unit_bounded());
    }
    else
    {
        return std::nullopt;
    }
}
}  // namespace plato::filter::extension
