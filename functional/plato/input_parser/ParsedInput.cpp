#include "plato/input_parser/ParsedInput.hpp"

#include "plato/input_parser/ComponentParserRegistration.hpp"
#include "plato/input_parser/GenericBlockRule.hpp"
#include "plato/utilities/StringUtilities.hpp"
#include "plato/utilities/TransformIf.hpp"

namespace plato::input_parser
{
namespace
{
template <ComponentType kComponentType, typename Iterator>
auto partition_by_component_type(const Iterator aBegin, const Iterator aEnd)
{
    return std::partition(aBegin, aEnd,
                          [](const auto& aInputBlock) { return aInputBlock.mComponentType == kComponentType; });
}

template <std::size_t... kIndices>
[[nodiscard]] auto partition_inputs_by_component_impl(std::vector<InputDataBlock>& aRawInput,
                                                      std::index_sequence<kIndices...>)
{
    using Iterator = typename std::vector<InputDataBlock>::iterator;
    auto tPartitionIterators = std::array<Iterator, sizeof...(kIndices) + 1>{};
    std::get<0>(tPartitionIterators) = aRawInput.begin();
    const auto tEnd = aRawInput.end();

    ((std::get<kIndices + 1>(tPartitionIterators) = partition_by_component_type<component_type_from_index<kIndices>()>(
          std::get<kIndices>(tPartitionIterators), tEnd)),
     ...);

    return tPartitionIterators;
}

template <typename IteratorArray, std::size_t... kIndices>
[[nodiscard]] auto partitioned_vector_to_array(const IteratorArray& aPartitionIterators,
                                               std::index_sequence<kIndices...>)
{
    auto tPartitionedInput = std::array<std::vector<InputDataBlock>, sizeof...(kIndices)>{};
    (std::move(std::get<kIndices>(aPartitionIterators), std::get<kIndices + 1>(aPartitionIterators),
               std::back_inserter(std::get<kIndices>(tPartitionedInput))),
     ...);
    return tPartitionedInput;
}

[[nodiscard]] auto partition_inputs_by_component(std::vector<InputDataBlock> aRawInput)
{
    constexpr auto tComponentIndices = std::make_index_sequence<utilities::number_of_enumerates<ComponentType>()>();
    const auto tPartitionIterators = partition_inputs_by_component_impl(aRawInput, tComponentIndices);
    return partitioned_vector_to_array(tPartitionIterators, tComponentIndices);
}

[[nodiscard]] auto parsed_input_or_errors(
    std::vector<ComponentBlockParser::ParsedDataOrError>&& aParsedComponentsOrError)
    -> utilities::Expected<ParsedInput, std::string>
{
    const auto tComponentHasParseError = [](const auto& aParsedComponentOrError)
    { return aParsedComponentOrError.hasError(); };

    if (std::any_of(aParsedComponentsOrError.begin(), aParsedComponentsOrError.end(), tComponentHasParseError))
    {
        auto tErrorMessages = std::vector<std::string>{};
        tErrorMessages.reserve(aParsedComponentsOrError.size());
        utilities::transform_if(
            aParsedComponentsOrError, std::back_inserter(tErrorMessages),
            [](const auto& aParsedComponentOrError) { return aParsedComponentOrError.error(); },
            tComponentHasParseError);
        return utilities::unexpected(utilities::concatenate_container(tErrorMessages, "\n"));
    }

    auto tParsedComponents = std::vector<InputDataBlock>{};
    tParsedComponents.reserve(aParsedComponentsOrError.size());
    std::transform(std::make_move_iterator(aParsedComponentsOrError.begin()),
                   std::make_move_iterator(aParsedComponentsOrError.end()), std::back_inserter(tParsedComponents),
                   [](ComponentBlockParser::ParsedDataOrError&& aComponent) { return std::move(aComponent).value(); });
    return ParsedInput{std::move(tParsedComponents)};
}

[[nodiscard]] auto parse_component(const std::unordered_map<std::string, ComponentBlockParser>& aComponentParsers,
                                   const GenericBlockData& aGenericBlock) -> ComponentBlockParser::ParsedDataOrError
{
    if (const auto tComponentParserIterator = aComponentParsers.find(aGenericBlock.mName.mToken);
        tComponentParserIterator != aComponentParsers.end())
    {
        return tComponentParserIterator->second.parse(aGenericBlock);
    }
    return utilities::unexpected("Parsing error: Could not find component with name: " + aGenericBlock.mName.mToken);
}

}  // namespace

ParsedInput::ParsedInput(std::vector<InputDataBlock> aRawInput)
    : mInputBlocks{partition_inputs_by_component(std::move(aRawInput))}
{
}

auto parse_to_new_input(const std::string& aInput) -> utilities::Expected<ParsedInput, std::string>
{
    return parse_to_new_input(aInput, registered_component_parsers());
}

auto parse_to_new_input(const std::string& aInput,
                        const std::unordered_map<std::string, ComponentBlockParser>& aComponentParsers)
    -> utilities::Expected<ParsedInput, std::string>
{
    const auto tGenericBlocksOrError = parse_generic_blocks(aInput);
    if (tGenericBlocksOrError.hasError())
    {
        return utilities::unexpected(tGenericBlocksOrError.error());
    }

    const auto& tGenericBlocks = tGenericBlocksOrError.value();
    auto tParsedInput = std::vector<ComponentBlockParser::ParsedDataOrError>{};
    std::transform(tGenericBlocks.begin(), tGenericBlocks.end(), std::back_inserter(tParsedInput),
                   [&aComponentParsers](const auto& aGenericBlock)
                   { return parse_component(aComponentParsers, aGenericBlock); });

    return parsed_input_or_errors(std::move(tParsedInput));
}

}  // namespace plato::input_parser
