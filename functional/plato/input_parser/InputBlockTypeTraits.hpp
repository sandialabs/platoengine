#ifndef PLATO_INPUT_PARSER_INPUTTYPETRAITS
#define PLATO_INPUT_PARSER_INPUTTYPETRAITS

#include "plato/input_parser/InputBlockStruct.hpp"

namespace plato::input_parser
{
/// @brief Helper struct to check whether type @a Input is a valid input block.
template <typename Input, typename = decltype(InputTypeName<Input>::name)>
struct IsInputBlock
{
    constexpr static bool value = true;
};

template <typename Input>
struct IsInputBlock<Input, void>
{
    constexpr static bool value = false;
};
}  // namespace plato::input_parser

#endif
