#ifndef PLATO_CORE_PROCESSMANAGER_EXTENSION_COMMONINPUTVALIDATION
#define PLATO_CORE_PROCESSMANAGER_EXTENSION_COMMONINPUTVALIDATION

#include <optional>
#include <string>

#include "plato/core/ValidationUtilities.hpp"

namespace plato::process_manager::extension::detail
{
template <typename InputBlock>
[[nodiscard]] std::optional<std::string> validate_number_of_steps(const InputBlock& aInput)
{
    namespace pfu = plato::utilities;
    return core::error_message_for_parameter_out_of_bounds(input_parser::block_name<InputBlock>(),
                                                           aInput.number_of_steps, "number_of_steps",
                                                           pfu::lower_bounded(pfu::Inclusive{1u}));
}

template <typename InputBlock>
std::optional<std::string> validate_initial_direction_magnitude(const InputBlock& aInput)
{
    namespace pfu = plato::utilities;
    return core::error_message_for_parameter_out_of_bounds(
        input_parser::block_name<InputBlock>(), aInput.initial_direction_magnitude, "initial_direction_magnitude",
        pfu::lower_bounded(pfu::Exclusive{0.0}));
}

template <typename InputBlock>
std::optional<std::string> validate_step_size_reduction_factor(const InputBlock& aInput)
{
    namespace pfu = plato::utilities;
    return core::error_message_for_parameter_out_of_bounds(
        input_parser::block_name<InputBlock>(), aInput.step_size_reduction_factor, "step_size_reduction_factor",
        pfu::ParameterBounds{pfu::Exclusive{0.0}, pfu::Exclusive{1.0}});
}

template <typename InputBlock>
std::optional<std::string> validate_random_direction_seed(const InputBlock& aInput)
{
    namespace pfu = plato::utilities;
    return core::error_message_for_parameter_out_of_bounds(input_parser::block_name<InputBlock>(),
                                                           aInput.random_direction_seed, "random_direction_seed",
                                                           pfu::lower_bounded(pfu::Inclusive{1u}));
}
}  // namespace plato::process_manager::extension::detail

#endif