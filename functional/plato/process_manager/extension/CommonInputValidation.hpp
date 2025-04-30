#ifndef PLATO_CORE_PROCESSMANAGER_EXTENSION_COMMONINPUTVALIDATION
#define PLATO_CORE_PROCESSMANAGER_EXTENSION_COMMONINPUTVALIDATION

#include <filesystem>
#include <optional>
#include <string>

#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_validation/ValidationUtilities.hpp"
#include "plato/utilities/StringUtilities.hpp"

namespace plato::process_manager::extension::detail
{

template <typename InputBlock>
[[nodiscard]] std::optional<std::string> validate_max_iterations(const InputBlock& aInput)
{
    return input_validation::error_message_for_optional_parameter_out_of_bounds(
        input_parser::block_name<InputBlock>(), aInput.max_iterations, "max_iterations",
        utilities::lower_bounded(utilities::Inclusive{1U}));
}

template <typename InputBlock>
[[nodiscard]] std::optional<std::string> validate_number_of_steps(const InputBlock& aInput)
{
    return input_validation::error_message_for_parameter_out_of_bounds(
        input_parser::block_name<InputBlock>(), aInput.number_of_steps, "number_of_steps",
        utilities::lower_bounded(utilities::Inclusive{1U}));
}

template <typename InputBlock>
std::optional<std::string> validate_initial_direction_magnitude(const InputBlock& aInput)
{
    namespace pfu = plato::utilities;
    return input_validation::error_message_for_parameter_out_of_bounds(
        input_parser::block_name<InputBlock>(), aInput.initial_direction_magnitude, "initial_direction_magnitude",
        pfu::lower_bounded(pfu::Exclusive{0.0}));
}

template <typename InputBlock>
std::optional<std::string> validate_step_size_reduction_factor(const InputBlock& aInput)
{
    namespace pfu = plato::utilities;
    return input_validation::error_message_for_parameter_out_of_bounds(
        input_parser::block_name<InputBlock>(), aInput.step_size_reduction_factor, "step_size_reduction_factor",
        pfu::ParameterBounds{pfu::Exclusive{0.0}, pfu::Exclusive{1.0}});
}

template <typename InputBlock>
std::optional<std::string> validate_random_direction_seed(const InputBlock& aInput)
{
    return input_validation::error_message_for_parameter_out_of_bounds(
        input_parser::block_name<InputBlock>(), aInput.random_direction_seed, "random_direction_seed",
        utilities::lower_bounded(utilities::Inclusive{1U}));
}

template <typename InputBlock>
std::optional<std::string> validate_optional_input_file_name(const InputBlock& aInput)
{
    if (aInput.input_file_name && !std::filesystem::exists(aInput.input_file_name->mToken))
    {
        return utilities::concatenate(input_parser::block_name<InputBlock>(), ": Could not find input file with name ",
                                      aInput.input_file_name->mToken);
    }
    return std::nullopt;
}

}  // namespace plato::process_manager::extension::detail

#endif
