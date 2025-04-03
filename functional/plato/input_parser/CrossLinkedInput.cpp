#include "plato/input_parser/CrossLinkedInput.hpp"

#include "plato/input_parser/ComponentParserRegistration.hpp"

namespace plato::input_parser
{
namespace
{
auto apply_cross_linker(const CrossLinker& aCrossLinker,
                        const NewParsedInput& aParsedInput,
                        std::vector<InputDataBlock>& aInputBlocksForSingleComponent,
                        std::vector<std::string>&& aErrorMessages) -> std::vector<std::string>
{
    for (auto& aInputBlock : aInputBlocksForSingleComponent)
    {
        auto tLinkedBlockOrError = aCrossLinker.crossLink(aInputBlock, aParsedInput);
        if (tLinkedBlockOrError.hasValue())
        {
            aInputBlock = std::move(tLinkedBlockOrError).value();
        }
        else
        {
            aErrorMessages.push_back(tLinkedBlockOrError.error());
        }
    }
    return std::move(aErrorMessages);
}

template <std::size_t... kComponentIndices>
auto apply_cross_linker(const CrossLinker& aCrossLinker,
                        NewParsedInput& aParsedInput,
                        std::vector<std::string>&& aErrorMessages,
                        std::index_sequence<kComponentIndices...>) -> std::vector<std::string>
{
    ((aErrorMessages = apply_cross_linker(aCrossLinker, aParsedInput,
                                          aParsedInput.get<component_type_from_index<kComponentIndices>()>(),
                                          std::move(aErrorMessages))),
     ...);
    return std::move(aErrorMessages);
}
}  // namespace

CrossLinkedInput::CrossLinkedInput(NewParsedInput aInput, const CrossLinkKey&) : mInput{std::move(aInput)} {}

auto CrossLinkedInput::rawInput() const -> const input_parser::NewParsedInput& { return mInput; };

auto make_cross_linked_input(input_parser::NewParsedInput aInput) -> utilities::Expected<CrossLinkedInput, std::string>
{
    auto tErrorMessages = std::vector<std::string>{};
    for (const auto& aCrossLinker : registered_cross_linkers())
    {
        tErrorMessages =
            apply_cross_linker(aCrossLinker, aInput, std::move(tErrorMessages),
                               std::make_index_sequence<utilities::number_of_enumerates<ComponentType>()>());
    }
    if (tErrorMessages.empty())
    {
        return CrossLinkedInput{std::move(aInput), CrossLinkKey{}};
    }
    return utilities::unexpected(utilities::concatenate_container(tErrorMessages, "\n"));
}
}  // namespace plato::input_parser
