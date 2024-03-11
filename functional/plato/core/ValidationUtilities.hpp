#ifndef PLATO_CORE_VALIDATIONUTILITIES
#define PLATO_CORE_VALIDATIONUTILITIES

#include <optional>
#include <string>
#include <vector>

#include "plato/input_parser/InputBlocks.hpp"
#include "plato/utilities/ParameterBounds.hpp"

namespace plato::core
{
/// @brief Creates a string by concatenating each entry of @a aMessages, with a newline between each.
[[nodiscard]] std::string all_messages(const std::vector<std::string>& aMessages);

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
        return std::string{aPrependString} + " entry \"" + std::string{aEntryName} + "\" has value " +
               std::to_string(aParameter.value()) + " and is outside the expected bounds " + aBounds.description();
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

}  // namespace plato::core

#endif
