#ifndef PLATO_INPUT_VALIDATION_VALIDATIONUTILITIES
#define PLATO_INPUT_VALIDATION_VALIDATIONUTILITIES

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_validation/ValidationRegistration.hpp"
#include "plato/utilities/ParameterBounds.hpp"
#include "plato/utilities/StringUtilities.hpp"

namespace plato::input_validation
{
/// @return an optional error message if @a aParameter does not contain a value.
template <typename T>
[[nodiscard]] auto error_message_for_empty_parameter(const std::string_view aPrependString,
                                                     const boost::optional<T>& aParameter,
                                                     const std::string_view aEntryName) -> std::optional<std::string>;

/// @return an optional error message if @a aParameter does not fall between @a aLowerBound and @a aUpperBound
template <typename T>
[[nodiscard]] auto error_message_for_parameter_out_of_bounds(const std::string_view aPrependString,
                                                             const boost::optional<T>& aParameter,
                                                             const std::string_view aEntryName,
                                                             const plato::utilities::ParameterBounds<T>& aBounds)
    -> std::optional<std::string>;

/// @return an optional error message if @a aParameter exists and does not fall between @a aLowerBound and @a
/// aUpperBound
/// No error message if aParameter doesn't exist
template <typename T>
[[nodiscard]] auto error_message_for_optional_parameter_out_of_bounds(
    const std::string_view aPrependString,
    const boost::optional<T>& aParameter,
    const std::string_view aEntryName,
    const plato::utilities::ParameterBounds<T>& aBounds) -> std::optional<std::string>;

/// @brief Checks if the objective or constraint given by @a aParameter should be included in the optimization problem.
/// @tparam Must have a public field `active` that is a `boost` or `std::optional`.
template <typename Parameter>
[[nodiscard]] auto is_active(const Parameter& aParameter) -> bool;

/// @return an optional error message if optional @a aParameter of type FileName does not contain a value or that value
/// doesn't point to a file on disk
[[nodiscard]] std::optional<std::string> error_message_for_missing_file_parameter(
    const std::string_view aPrependString,
    const boost::optional<input_parser::FileName>& aParameter,
    const std::string_view aEntryName);

/// @return an optional error message if the file retrieved from the input @a aInput using accessor @a aFileNameAccessor
/// doesn't point to a file on disk.
template <typename InputBlock, typename FileNameAccessor>
[[nodiscard]] auto error_message_for_missing_file_on_disk(const InputBlock& aInput,
                                                          const FileNameAccessor& aFileNameAccessor,
                                                          const std::string_view aEntryName)
    -> std::optional<std::string>;

template <typename T>
auto error_message_for_empty_parameter(const std::string_view aPrependString,
                                       const boost::optional<T>& aParameter,
                                       const std::string_view aEntryName) -> std::optional<std::string>
{
    if (!aParameter)
    {
        return utilities::concatenate(aPrependString, " missing required entry \"", aEntryName, "\"");
    }
    else
    {
        return std::nullopt;
    }
}

template <typename T>
auto error_message_for_parameter_out_of_bounds(const std::string_view aPrependString,
                                               const boost::optional<T>& aParameter,
                                               const std::string_view aEntryName,
                                               const plato::utilities::ParameterBounds<T>& aBounds)
    -> std::optional<std::string>
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

template <typename T>
auto error_message_for_optional_parameter_out_of_bounds(const std::string_view aPrependString,
                                                        const boost::optional<T>& aParameter,
                                                        const std::string_view aEntryName,
                                                        const plato::utilities::ParameterBounds<T>& aBounds)
    -> std::optional<std::string>
{
    if (aParameter.has_value())
    {
        return input_validation::error_message_for_parameter_out_of_bounds(aPrependString, aParameter, aEntryName,
                                                                           aBounds);
    }
    else
    {
        return std::nullopt;
    }
}

template <typename Parameter>
bool is_active(const Parameter& aParameter)
{
    return !aParameter.active.has_value() || aParameter.active.value();
}

template <typename InputBlock, typename FileNameAccessor>
[[nodiscard]] auto error_message_for_missing_file_on_disk(const InputBlock& aInput,
                                                          const FileNameAccessor& aFileNameAccessor,
                                                          const std::string_view aEntryName)
    -> std::optional<std::string>
{
    const auto& tFileName = aFileNameAccessor(aInput);
    if (tFileName.has_value() && !std::filesystem::exists(tFileName.value().mToken))
    {
        return input_parser::block_name<InputBlock>() + ": The " + std::string{aEntryName} +
               " entry does not refer to a valid input file. The entered path is " + tFileName.value().mToken;
    }
    return std::nullopt;
}

}  // namespace plato::input_validation

#endif
