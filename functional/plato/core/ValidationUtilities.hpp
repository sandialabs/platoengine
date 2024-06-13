#ifndef PLATO_CORE_VALIDATIONUTILITIES
#define PLATO_CORE_VALIDATIONUTILITIES

#include <optional>
#include <string>
#include <vector>

#include "plato/core/InputVariantUtilities.hpp"
#include "plato/core/ValidationRegistration.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/utilities/ParameterBounds.hpp"
#include "plato/utilities/StringUtilities.hpp"

namespace plato::core
{
/// @return an optional error message if @a aParameter does not contain a value.
template <typename T>
[[nodiscard]] std::optional<std::string> error_message_for_empty_parameter(const std::string_view aPrependString,
                                                                           const boost::optional<T>& aParameter,
                                                                           const std::string_view aEntryName);

/// @return an optional error message if @a aParameter does not fall between @a aLowerBound and @a aUpperBound
template <typename T>
[[nodiscard]] std::optional<std::string> error_message_for_parameter_out_of_bounds(
    const std::string_view aPrependString,
    const boost::optional<T>& aParameter,
    const std::string_view aEntryName,
    const plato::utilities::ParameterBounds<T>& aBounds);

/// @brief Checks if the objective or constraint given by @a aParameter should be included in the optimization problem.
/// @tparam Must have a public field `active` that is a `boost` or `std::optional`.
template <typename Parameter>
[[nodiscard]] bool is_active(const Parameter& aParameter);

/// @brief Calls the registered validation functions on all input blocks in @a aInput for
///  the input block types contained in @a InputVariant.
template <typename InputVariant>
[[nodiscard]] std::vector<std::string> validate_all_variants(const input_parser::ParsedInput& aInput,
                                                             std::vector<std::string>&& aCurrentMessageList);

template <typename T>
std::optional<std::string> error_message_for_empty_parameter(const std::string_view aPrependString,
                                                             const boost::optional<T>& aParameter,
                                                             const std::string_view aEntryName)
{
    if (!aParameter)
    {
        return std::string(aPrependString) + " missing required entry \"" + std::string{aEntryName} + "\"";
    }
    else
    {
        return std::nullopt;
    }
}

template <typename T>
std::optional<std::string> error_message_for_parameter_out_of_bounds(
    const std::string_view aPrependString,
    const boost::optional<T>& aParameter,
    const std::string_view aEntryName,
    const plato::utilities::ParameterBounds<T>& aBounds)
{
    if (aParameter && !aBounds.contains(aParameter.value()))
    {
        return utilities::concatenate(aPrependString, " entry \"", aEntryName, "\" has value ", aParameter.value(),
                                      " and is outside the expected bounds ", aBounds.description());
    }
    else
    {
        return error_message_for_empty_parameter(aPrependString, aParameter, aEntryName);
    }
}

template <typename Parameter>
bool is_active(const Parameter& aParameter)
{
    return !aParameter.active.has_value() || aParameter.active.value();
}

template <typename InputVariant>
std::vector<std::string> validate_all_variants(const input_parser::ParsedInput& aInput,
                                               std::vector<std::string>&& aCurrentMessageList)
{
    const std::vector<InputVariant> tInputBlocks = core::all_input_blocks_in_variant<InputVariant>(aInput);
    for (const InputVariant& tBlockEntry : tInputBlocks)
    {
        aCurrentMessageList = std::visit(
            [tList = std::move(aCurrentMessageList)](const auto& aVariantInput) mutable -> std::vector<std::string>
            {
                return core::validate(aVariantInput, std::move(tList));  // NOLINT
            },
            tBlockEntry);
    }
    return std::move(aCurrentMessageList);
}

}  // namespace plato::core

#endif
