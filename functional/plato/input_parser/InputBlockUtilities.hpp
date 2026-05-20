#ifndef PLATO_INPUT_PARSER_INPUT_BLOCK_UTILITIES
#define PLATO_INPUT_PARSER_INPUT_BLOCK_UTILITIES

#include "plato/input_parser/InputBlockTypeTraits.hpp"
#include "plato/input_parser/ParsedInput.hpp"

namespace plato::input_parser
{
/// @brief Returns the name of an input block, as it appears in the input deck
template <typename BlockStruct>
[[nodiscard]] auto block_name() -> std::string;

/// @brief Overloaded pipe operator for composing input blocks into a full ParsedInput struct.
template <typename Input>
[[nodiscard]] auto operator|(ParsedInput aParsedInput,
                             const Input& aInput) -> std::enable_if_t<IsInputBlock<Input>::mValue, ParsedInput>;

/// @brief Overload to construct initial empty ParsedInput struct.
template <typename InputOne, typename InputTwo>
    requires(IsInputBlock<InputOne>::mValue && IsInputBlock<InputTwo>::mValue)
[[nodiscard]] auto operator|(const InputOne& aInputOne, const InputTwo& aInputTwo) -> ParsedInput;

template <typename BlockStruct>
auto block_name() -> std::string
{
    return InputTypeName<BlockStruct>::mName;
}

template <typename Input>
[[nodiscard]] auto operator|(ParsedInput aParsedInput,
                             const Input& aInput) -> std::enable_if_t<IsInputBlock<Input>::mValue, ParsedInput>
{
    constexpr auto tComponentType = ComponentTypeOfInputBlock<Input>::mValue;
    aParsedInput.get<tComponentType>().push_back(
        InputDataBlock{tComponentType, block_name<Input>(), InputBlockWrapper{aInput}});
    return aParsedInput;
}

template <typename InputOne, typename InputTwo>
    requires(IsInputBlock<InputOne>::mValue && IsInputBlock<InputTwo>::mValue)
[[nodiscard]] auto operator|(const InputOne& aInputOne, const InputTwo& aInputTwo) -> ParsedInput
{
    return ParsedInput{} | aInputOne | aInputTwo;
}

}  // namespace plato::input_parser

#endif
