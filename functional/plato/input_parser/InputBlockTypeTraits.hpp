#ifndef PLATO_INPUT_PARSER_TYPE_OR_VECTOR_VALUE_TYPE
#define PLATO_INPUT_PARSER_TYPE_OR_VECTOR_VALUE_TYPE

#include <boost/optional.hpp>
#include <vector>

#include "plato/input_parser/InputBlockStruct.hpp"

namespace plato::input_parser
{
/// Helper to provide the `value_type` of a std::vector or the type T if it is not a std::vector.
template <typename T>
struct TypeOrVectorValueType
{
    using type = T;
};

template <typename T, typename A>
struct TypeOrVectorValueType<std::vector<T, A>>
{
    using type = typename std::vector<T, A>::value_type;
};

template <typename T>
struct TypeOrVectorValueType<boost::optional<T>>
{
    using type = T;
};

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