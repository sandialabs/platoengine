#ifndef PLATO_INPUT_PARSER_INPUT_BLOCK_UTILITIES
#define PLATO_INPUT_PARSER_INPUT_BLOCK_UTILITIES

#include "plato/input_parser/InputBlockTypeTraits.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/input_parser/ParsedInput.hpp"

namespace plato::input_parser
{
/// @brief Overloaded pipe operator for composing input blocks into a full ParsedInput struct.
template <typename Input>
[[nodiscard]] auto operator|(NewParsedInput aParsedInput, const Input& aInput)
    -> std::enable_if_t<IsInputBlock<Input>::value, NewParsedInput>;

/// @brief Overload to construct initial empty ParsedInput struct.
template <typename InputOne, typename InputTwo>
[[nodiscard]] auto operator|(const InputOne& aInputOne, const InputTwo& aInputTwo)
    -> std::enable_if_t<IsInputBlock<InputOne>::value && IsInputBlock<InputTwo>::value, NewParsedInput>;

template <typename Input>
[[nodiscard]] auto operator|(NewParsedInput aParsedInput, const Input& aInput)
    -> std::enable_if_t<IsInputBlock<Input>::value, NewParsedInput>
{
    constexpr auto tComponentType = ComponentTypeOfInputBlock<Input>::value;
    aParsedInput.get<tComponentType>().push_back(
        InputDataBlock{tComponentType, block_name<Input>(), CrossReferencedInput{aInput}});
    return aParsedInput;
}

template <typename InputOne, typename InputTwo>
[[nodiscard]] auto operator|(const InputOne& aInputOne, const InputTwo& aInputTwo)
    -> std::enable_if_t<IsInputBlock<InputOne>::value && IsInputBlock<InputTwo>::value, NewParsedInput>
{
    return NewParsedInput{} | aInputOne | aInputTwo;
}

}  // namespace plato::input_parser

#endif
