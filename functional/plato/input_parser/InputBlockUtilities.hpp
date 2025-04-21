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
[[nodiscard]] auto operator|(ParsedInput aParsedInput, const Input& aInput)
    -> std::enable_if_t<IsInputBlock<Input>::value, ParsedInput>;

/// @brief Overload to construct initial empty ParsedInput struct.
template <typename InputOne, typename InputTwo>
[[nodiscard]] auto operator|(const InputOne& aInputOne, const InputTwo& aInputTwo)
    -> std::enable_if_t<IsInputBlock<InputOne>::value && IsInputBlock<InputTwo>::value, ParsedInput>;

template <typename BlockStruct>
auto block_name() -> std::string
{
    return InputTypeName<BlockStruct>::name;
}

template <typename Input>
[[nodiscard]] auto operator|(ParsedInput aParsedInput, const Input& aInput)
    -> std::enable_if_t<IsInputBlock<Input>::value, ParsedInput>
{
    constexpr auto tComponentType = ComponentTypeOfInputBlock<Input>::value;
    aParsedInput.get<tComponentType>().push_back(
        InputDataBlock{tComponentType, block_name<Input>(), InputBlockWrapper{aInput}});
    return aParsedInput;
}

template <typename InputOne, typename InputTwo>
[[nodiscard]] auto operator|(const InputOne& aInputOne, const InputTwo& aInputTwo)
    -> std::enable_if_t<IsInputBlock<InputOne>::value && IsInputBlock<InputTwo>::value, ParsedInput>
{
    return ParsedInput{} | aInputOne | aInputTwo;
}

}  // namespace plato::input_parser

#endif
