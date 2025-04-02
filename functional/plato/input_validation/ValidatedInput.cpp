#include "plato/input_validation/ValidatedInput.hpp"

#include "plato/input_validation/ValidationRegistration.hpp"
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
[[nodiscard]] auto validate_components(const input_parser::NewParsedInput& aInput, std::index_sequence<kIndices...>)
    -> std::vector<std::string>
{
    auto tErrorMessages = std::vector<std::string>{};
    ((tErrorMessages = append_validation_errors(std::move(tErrorMessages),
                                                aInput.get<input_parser::component_type_from_index<kIndices>()>())),
     ...);
    return tErrorMessages;
}
}  // namespace

ValidatedInput::ValidatedInput(const input_parser::NewParsedInput& /*aInput*/, const ValidateKey&) {}

auto make_validated_input(const input_parser::NewParsedInput& aInput)
    -> utilities::Expected<ValidatedInput, std::string>
{
    auto tErrorMessages = validate_components(
        aInput, std::make_index_sequence<utilities::number_of_enumerates<input_parser::ComponentType>()>());
    tErrorMessages = validate(aInput, std::move(tErrorMessages));

    if (tErrorMessages.empty())
    {
        return ValidatedInput{aInput, ValidateKey{}};
    }
    return utilities::unexpected(utilities::concatenate_container(tErrorMessages, "\n"));
}
}  // namespace plato::input_validation
