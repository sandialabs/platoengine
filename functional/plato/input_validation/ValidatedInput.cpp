#include "plato/input_validation/ValidatedInput.hpp"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "plato/components/ComponentType.hpp"
#include "plato/input_parser/CrossLinkedInput.hpp"
#include "plato/input_parser/InputBlockData.hpp"
#include "plato/input_parser/ParsedInput.hpp"
#include "plato/input_validation/ValidationRegistration.hpp"
#include "plato/utilities/EnumIndexing.hpp"
#include "plato/utilities/Expected.hpp"
#include "plato/utilities/StringUtilities.hpp"

namespace plato::input_validation
{
namespace
{
auto append_validation_errors(std::vector<std::string>&& aErrorMessages,
                              const std::vector<input_parser::InputDataBlock>& aInputDataBlocks)
    -> std::vector<std::string>
{
    for (const auto& aInputDataBlock : aInputDataBlocks)
    {
        aErrorMessages = validate(aInputDataBlock.mInput, std::move(aErrorMessages));
    }
    return std::move(aErrorMessages);
}

template <std::size_t... kIndices>
[[nodiscard]] auto validate_components(const input_parser::ParsedInput& aInput, std::index_sequence<kIndices...>)
    -> std::vector<std::string>
{
    auto tErrorMessages = std::vector<std::string>{};
    ((tErrorMessages = append_validation_errors(std::move(tErrorMessages),
                                                aInput.get<components::component_type_from_index<kIndices>()>())),
     ...);
    return tErrorMessages;
}
}  // namespace

/// @brief Key for implementing the pass-key idiom for ValidatedInput. The key is only constructible by the function
/// make_validated_input.
struct ValidateKey
{
    friend auto make_validated_input(const input_parser::CrossLinkedInput& input)
        -> utilities::Expected<ValidatedInput, std::string>;

    ~ValidateKey() = default;

   private:
    ValidateKey() = default;
    ValidateKey(const ValidateKey&) = default;
    ValidateKey(ValidateKey&&) = default;
    auto operator=(const ValidateKey&) -> ValidateKey& = default;
    auto operator=(ValidateKey&&) -> ValidateKey& = default;
};

ValidatedInput::ValidatedInput(input_parser::ParsedInput aInput, const ValidateKey&) : mRawInput{std::move(aInput)} {}

auto parse_and_validate_string(const std::string_view aInput) -> utilities::Expected<ValidatedInput, std::string>
{
    auto tParsedInput = input_parser::make_parsed_input(std::string{aInput});
    if (tParsedInput.hasError())
    {
        return utilities::unexpected(tParsedInput.error());
    }
    auto tCrossLinkedInput = make_cross_linked_input(std::move(tParsedInput).value());
    if (tCrossLinkedInput.hasError())
    {
        return utilities::unexpected(tCrossLinkedInput.error());
    }
    return make_validated_input(tCrossLinkedInput.value());
}

auto parse_and_validate_file(const std::filesystem::path& aFileName) -> utilities::Expected<ValidatedInput, std::string>
{
    auto tInputStream = std::ifstream{aFileName};
    const auto tInputFileString =
        std::string((std::istreambuf_iterator<char>(tInputStream)), std::istreambuf_iterator<char>());
    return parse_and_validate_string(tInputFileString);
}

auto make_validated_input(const input_parser::CrossLinkedInput& aInput)
    -> utilities::Expected<ValidatedInput, std::string>
{
    auto tErrorMessages = validate_components(
        aInput.rawInput(), std::make_index_sequence<utilities::number_of_enumerates<components::ComponentType>()>());
    tErrorMessages = validate(aInput.rawInput(), std::move(tErrorMessages));

    if (tErrorMessages.empty())
    {
        return ValidatedInput{aInput.rawInput(), ValidateKey{}};
    }
    return utilities::unexpected(utilities::concatenate_container(tErrorMessages, "\n"));
}

auto make_validated_input(const input_parser::ParsedInput& aInput) -> utilities::Expected<ValidatedInput, std::string>
{
    const auto tCrossLinkedInputOrError = input_parser::make_cross_linked_input(aInput);
    if (tCrossLinkedInputOrError.hasError())
    {
        return utilities::unexpected(tCrossLinkedInputOrError.error());
    }
    return make_validated_input(tCrossLinkedInputOrError.value());
}

}  // namespace plato::input_validation
