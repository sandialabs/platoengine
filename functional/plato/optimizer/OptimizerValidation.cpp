#include "plato/optimizer/OptimizerValidation.hpp"

#include "plato/core/ValidationRegistration.hpp"
#include "plato/core/ValidationUtilities.hpp"
#include "plato/input_parser/InputBlocks.hpp"

namespace plato::optimizer
{
[[maybe_unused]] static auto kOptimizerValidationRegistration =
    core::ValidationRegistration<input_parser::optimization_parameters>{
        [](const input_parser::optimization_parameters& aInput) { return detail::validate_max_iterations(aInput); },
        [](const input_parser::optimization_parameters& aInput) { return detail::validate_step_tolerance(aInput); },
        [](const input_parser::optimization_parameters& aInput)
        { return detail::validate_gradient_tolerance(aInput); }};

std::vector<std::string> validate_optimization_parameters(const input_parser::optimization_parameters& aInput,
                                                          std::vector<std::string>&& aCurrentMessageList)
{
    return core::validate(aInput, std::move(aCurrentMessageList));
}

namespace detail
{
std::optional<std::string> validate_max_iterations(const input_parser::optimization_parameters& aInput)
{
    namespace pfu = plato::utilities;
    if (!aInput.input_file_name || (aInput.input_file_name && aInput.max_iterations))
    {
        return core::error_message_for_parameter_out_of_bounds(
            input_parser::block_name<input_parser::optimization_parameters>(), aInput.max_iterations, "max_iterations",
            pfu::lower_bounded(pfu::Inclusive{1u}));
    }
    else
    {
        return std::nullopt;
    }
}

[[nodiscard]] std::optional<std::string> validate_step_tolerance(const input_parser::optimization_parameters& aInput)
{
    namespace pfu = plato::utilities;
    if (!aInput.input_file_name || (aInput.input_file_name && aInput.step_tolerance))
    {
        return core::error_message_for_parameter_out_of_bounds(
            input_parser::block_name<input_parser::optimization_parameters>(), aInput.step_tolerance, "step_tolerance",
            pfu::lower_bounded(pfu::Exclusive{0.0}));
    }
    else
    {
        return std::nullopt;
    }
}

[[nodiscard]] std::optional<std::string> validate_gradient_tolerance(
    const input_parser::optimization_parameters& aInput)
{
    namespace pfu = plato::utilities;
    if (!aInput.input_file_name || (aInput.input_file_name && aInput.gradient_tolerance))
    {
        return core::error_message_for_parameter_out_of_bounds(
            input_parser::block_name<input_parser::optimization_parameters>(), aInput.gradient_tolerance,
            "gradient_tolerance", pfu::lower_bounded(pfu::Exclusive{0.0}));
    }
    else
    {
        return std::nullopt;
    }
}

}  // namespace detail
}  // namespace plato::optimizer
