#ifndef PLATO_INPUTBLOCKSTRUCT_HPP
#define PLATO_INPUTBLOCKSTRUCT_HPP

#include <boost/fusion/adapted/struct/define_struct.hpp>
#include <boost/fusion/include/define_struct.hpp>

#include <boost/preprocessor/seq/push_front.hpp>
#include <boost/preprocessor/seq/transform.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/seq/variadic_seq_to_seq.hpp>

#include <boost/optional.hpp>

namespace Plato::Input{

/// @brief Provides the name of an input block parsed into @a InputStruct as a static `name` member.
/// @note The actual implementation is via template specializations generated from the macros.
template<typename InputStruct>
struct InputTypeName{};

/// @brief Specifies whether the input block parsed into @a InputStruct is named or unnamed.
/// @note The actual implementation is via template specializations generated from the macros.
template<typename InputStruct>
constexpr bool kIsNamedBlock = false;
}

#define OPTIONAL_TYPE(r, data, elem) boost::optional<BOOST_PP_TUPLE_ELEM(0, elem)>, BOOST_PP_TUPLE_ELEM(1, elem)
#define PLATO_BOOST_FUSION_NAME_FIELD (std::string, name)
#define ATTRIBUTES_WITH_NAME(ATTRIBUTES) BOOST_PP_SEQ_PUSH_FRONT(ATTRIBUTES, PLATO_BOOST_FUSION_NAME_FIELD)
#define TYPES_AS_OPTIONAL(ATTRIBUTES) BOOST_PP_SEQ_TRANSFORM(OPTIONAL_TYPE, _, ATTRIBUTES)

/// Macro for generating an adapted struct that can be used for input parsing. The format
/// is the same as BOOST_FUSION_DEFINE_STRUCT and the resulting struct has all the same
/// properties.
///
/// For example, a struct of the form:
/// @code 
/// PLATO_INPUT_BLOCK_STRUCT(
///    (Plato), service,
///    (unsigned int, number_processors))
/// @endcode
/// parses the following block:
/// @code
/// begin service my_service
///   number_processors 10
/// end service
/// @endcode
#define PLATO_INPUT_BLOCK_STRUCT(NAMESPACE_SEQ, STRUCT_NAME, ATTRIBUTES)                       \
BOOST_FUSION_DEFINE_STRUCT(                                                                    \
    NAMESPACE_SEQ,                                                                             \
    STRUCT_NAME,                                                                               \
    TYPES_AS_OPTIONAL(BOOST_PP_VARIADIC_SEQ_TO_SEQ(ATTRIBUTES))                                \
)                                                                                              \
namespace Plato::Input{                                                                        \
template<>                                                                                     \
struct InputTypeName<STRUCT_NAME>                                                              \
{   static constexpr const char* name = #STRUCT_NAME;                                          \
};                                                                                             \
}

/// Macro for generating an adapted struct that can be used for input parsing. The format
/// is the same as BOOST_FUSION_DEFINE_STRUCT and the resulting struct has all the same
/// properties. This will include a `name` field used to parse a block name.
///
/// For example, a struct of the form:
/// @code 
/// PLATO_NAMED_INPUT_BLOCK_STRUCT(
///    (Plato), service,
///    (unsigned int, number_processors))
/// @endcode
/// parses the following block:
/// @code
/// begin service my_service
///   number_processors 10
/// end service
/// @endcode
#define PLATO_NAMED_INPUT_BLOCK_STRUCT(NAMESPACE_SEQ, STRUCT_NAME, ATTRIBUTES)        \
BOOST_FUSION_DEFINE_STRUCT(                                                           \
    NAMESPACE_SEQ,                                                                    \
    STRUCT_NAME,                                                                      \
    TYPES_AS_OPTIONAL(ATTRIBUTES_WITH_NAME(BOOST_PP_VARIADIC_SEQ_TO_SEQ(ATTRIBUTES))) \
)                                                                                     \
namespace Plato::Input{                                                               \
template<>                                                                            \
struct InputTypeName<STRUCT_NAME>                                                     \
{   static constexpr const char* name = #STRUCT_NAME;                                 \
};                                                                                    \
template<>                                                                            \
constexpr bool kIsNamedBlock<STRUCT_NAME> = true;                                     \
}

#endif
