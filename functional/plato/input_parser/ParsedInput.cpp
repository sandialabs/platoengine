#include "plato/input_parser/ParsedInput.hpp"

#include "plato/input_parser/GenericBlockRule.hpp"

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

template <std::size_t kIndex>
[[nodiscard]] constexpr auto component_type_from_index() -> ComponentType
{
    return std::get<0>(utilities::enums_from_index<ComponentType>(kIndex));
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

}  // namespace

NewParsedInput::NewParsedInput(std::vector<InputDataBlock> aRawInput)
    : mInputBlocks{partition_inputs_by_component(std::move(aRawInput))}
{
}

auto parse_to_new_input(const std::string& aInput,
                        const std::unordered_map<std::string, ComponentBlockParser>& aComponentParsers)
    -> NewParsedInput
{
    const auto tGenericBlocks = parse_generic_blocks(aInput);
    auto tParsedInput = std::vector<InputDataBlock>{};
    std::transform(tGenericBlocks.begin(), tGenericBlocks.end(), std::back_inserter(tParsedInput),
                   [&aComponentParsers](const auto& aGenericBlock)
                   { return aComponentParsers.at(aGenericBlock.mName.mToken).parse(aGenericBlock); });
    return NewParsedInput{std::move(tParsedInput)};
}

}  // namespace plato::input_parser
