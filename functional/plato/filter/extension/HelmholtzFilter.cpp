#include "plato/filter/extension/HelmholtzFilter.hpp"

#include <filesystem>
#include <memory>

#include "plato/core/Function.hpp"
#include "plato/core/MeshProxy.hpp"
#include "plato/core/ValidationRegistration.hpp"
#include "plato/core/ValidationUtilities.hpp"
#include "plato/filter/extension/CommonInputValidation.hpp"
#include "plato/filter/library/FilterInterface.hpp"
#include "plato/filter/library/FilterJacobian.hpp"
#include "plato/filter/library/FilterRegistration.hpp"

namespace plato::filter::extension
{
namespace
{
const auto kHelmholtzFilterLibName = std::filesystem::path{"libAnalyzeFunctionalInterface.so"};

template <typename T>
std::optional<T> to_std_optional(const boost::optional<T>& aT)
{
    if (aT)
    {
        return aT.value();
    }
    else
    {
        return std::nullopt;
    }
}

library::FilterParameters to_filter_parameters(const input_parser::helmholtz_filter& aInput)
{
    return library::FilterParameters{/*.mFilterRadius=*/aInput.filter_radius.value(),
                                     /*.mBoundaryStickingPenalty=*/to_std_optional(aInput.boundary_sticking_penalty)};
}

auto make_filter_function_from_interface(std::unique_ptr<library::FilterInterface> aFilter) -> library::FilterFunction
{
    auto tFilterAsShared = std::shared_ptr<library::FilterInterface>(std::move(aFilter));
    return core::make_function([tFilterAsShared](const core::MeshProxy& aMeshProxy)
                               { return tFilterAsShared->filter(aMeshProxy); },
                               [tFilterAsShared](const core::MeshProxy& aMeshProxy) {
                                   return library::FilterJacobian{tFilterAsShared, aMeshProxy};
                               });
}

[[maybe_unused]] static auto kHelmholtzFilterRegistration = library::FilterRegistration{
    input_parser::block_name<input_parser::helmholtz_filter>(), [](const library::ValidatedFilterInput& aInput)
    {
        const auto& tInput = core::validated_variant_raw_input<input_parser::helmholtz_filter>(aInput);
        return make_filter_function_from_interface(
            library::load_filter(to_filter_parameters(tInput), kHelmholtzFilterLibName));
    }};

[[maybe_unused]] static auto kHelmholtzFilterValidationRegistration =
    core::ValidationRegistration<input_parser::helmholtz_filter>{
        [](const input_parser::helmholtz_filter& aInput) { return detail::validate_filter_radius(aInput); },
        [](const input_parser::helmholtz_filter& aInput)
        { return validate_helmholtz_filter_boundary_sticking_penalty(aInput); }};
}  // namespace

[[nodiscard]] std::optional<std::string> validate_helmholtz_filter_boundary_sticking_penalty(
    const input_parser::helmholtz_filter& aInput)
{
    namespace pfu = plato::utilities;
    return core::error_message_for_optional_parameter_out_of_bounds(
        input_parser::block_name<input_parser::helmholtz_filter>(), aInput.boundary_sticking_penalty,
        "boundary_sticking_penalty", pfu::unit_bounded());
}
}  // namespace plato::filter::extension
