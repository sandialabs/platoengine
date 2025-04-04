#ifndef PLATO_INPUT_PARSER_INPUT_BLOCK_UTILITIES
#define PLATO_INPUT_PARSER_INPUT_BLOCK_UTILITIES

#include <boost/fusion/algorithm.hpp>
#include <boost/mpl/arg.hpp>
#include <boost/type_traits.hpp>

#include "plato/input_parser/InputBlockTypeTraits.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/input_parser/ParsedInput.hpp"

namespace plato::input_parser
{
/// @brief Overloaded pipe operator for composing input blocks into a full ParsedInput struct.
template <typename Input>
[[nodiscard]] auto operator|(NewParsedInput&& aParsedInput, const Input& aInput)
    -> std::enable_if_t<IsInputBlock<Input>::value, NewParsedInput>;

/*
/// @brief Overload to construct initial empty ParsedInput struct.
template <typename InputOne, typename InputTwo>
[[nodiscard]] auto operator|(const InputOne& aInputOne, const InputTwo& aInputTwo)
    -> std::enable_if_t<IsInputBlock<InputOne>::value && IsInputBlock<InputTwo>::value, NewParsedInput>
{
    return NewParsedInput{{}} | aInputOne | aInputTwo;
}
*/
/// @brief Overloaded pipe operator for composing input blocks into a full ParsedInput struct.
template <typename Input>
[[nodiscard]] ParsedInput operator|(ParsedInput&& aParsedInput, const Input& aInput)
{
    boost::fusion::for_each(aParsedInput,
                            [aInput](auto& field)
                            {
                                using FieldType = std::decay_t<decltype(field)>;
                                using FieldBaseType = typename TypeOrVectorValueType<FieldType>::type;

                                if constexpr (std::is_same_v<Input, FieldBaseType>)
                                {
                                    field = FieldType{aInput};
                                }
                            });

    return std::move(aParsedInput);
}

/// @brief Overload to construct initial empty ParsedInput struct.
template <typename InputOne, typename InputTwo>
[[nodiscard]] auto operator|(const InputOne& aInputOne, const InputTwo& aInputTwo)
    -> std::enable_if_t<IsInputBlock<InputOne>::value && IsInputBlock<InputTwo>::value, ParsedInput>
{
    return ParsedInput{} | aInputOne | aInputTwo;
}

template <typename Input>
[[nodiscard]] auto operator|(NewParsedInput&& aParsedInput, const Input& aInput)
    -> std::enable_if_t<IsInputBlock<Input>::value, NewParsedInput>
{
    constexpr auto tComponentType = ComponentTypeOfInputBlock<Input>::value;
    aParsedInput.get<tComponentType>().push_back(
        InputDataBlock{tComponentType, block_name<Input>(), CrossReferencedInput{aInput}});
    return aParsedInput;
}

}  // namespace plato::input_parser

#endif
