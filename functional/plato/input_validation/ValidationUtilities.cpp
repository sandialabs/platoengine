#include "plato/input_validation/ValidationUtilities.hpp"

#include <filesystem>

namespace plato::input_validation
{
std::optional<std::string> error_message_for_missing_file_parameter(
    const std::string_view aPrependString,
    const boost::optional<input_parser::FileName>& aParameter,
    const std::string_view aEntryName)
{
    if (aParameter)
    {
        const auto tExists = std::filesystem::exists(std::string{aParameter.value().mToken});
        if (!tExists)
        {
            return utilities::concatenate(aPrependString, " specified file '", aParameter.value().mToken,
                                          "' does not exist on disk.");
        }
        return std::nullopt;
    }
    return error_message_for_empty_parameter(aPrependString, aParameter, aEntryName);
}
}  // namespace plato::input_validation
