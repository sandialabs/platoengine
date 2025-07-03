#ifndef PLATO_INPUT_PARSER_INPUTBLOCKSTRUCT
#define PLATO_INPUT_PARSER_INPUTBLOCKSTRUCT

#include <boost/fusion/adapted/struct/define_struct.hpp>
#include <boost/fusion/include/define_struct.hpp>
#include <boost/optional.hpp>
#include <boost/preprocessor/seq/push_front.hpp>
#include <boost/preprocessor/seq/seq.hpp>
#include <boost/preprocessor/seq/transform.hpp>
#include <boost/preprocessor/seq/variadic_seq_to_seq.hpp>
#include <boost/preprocessor/tuple/elem.hpp>

#include "plato/components/ComponentType.hpp"
#include "plato/input_parser/HelpDocumentationRegistration.hpp"

namespace plato::input_parser
{
/// @brief Provides the name of an input block parsed into @a InputStruct as a static `name` member.
/// @note The actual implementation is via template specializations generated from the macros.
template <typename InputStruct>
struct InputTypeName
{
};

/// @brief A type trait with a member variable corresponding to the components::ComponentType of @a T.
///
/// This is specialized for each input block type in the defining macros.
template <typename T>
struct ComponentTypeOfInputBlock
{
};

/// @brief Specifies whether the input block parsed into @a InputStruct is named or unnamed.
/// @note The actual implementation is via template specializations generated from the macros.
template <typename InputStruct>
constexpr inline bool kIsNamedBlock = false;

/// @brief A helper function template for registering HelpDocumentation object generation.
/// @note This is specialized in the parser macros and does not need to be called by client code.
template <typename InputStruct>
void register_help_documentation();
}  // namespace plato::input_parser

// clang-format off
#define OPTIONAL_TYPE(r, data, elem) boost::optional<BOOST_PP_TUPLE_ELEM(0, elem)>, BOOST_PP_TUPLE_ELEM(1, elem)
#define PLATO_BOOST_FUSION_NAME_FIELD (std::string, name)
#define ATTRIBUTES_WITH_NAME(ATTRIBUTES) BOOST_PP_SEQ_PUSH_FRONT(ATTRIBUTES, PLATO_BOOST_FUSION_NAME_FIELD)
#define TYPES_AS_OPTIONAL(ATTRIBUTES) BOOST_PP_SEQ_TRANSFORM(OPTIONAL_TYPE, _, ATTRIBUTES)

#define STRIP_COMMENT(r, data, elem) (BOOST_PP_TUPLE_ELEM(0, elem), BOOST_PP_TUPLE_ELEM(1, elem))
#define ATTRIBUTES_WITHOUT_COMMENTS(ATTRIBUTES) BOOST_PP_SEQ_TRANSFORM(STRIP_COMMENT, _, ATTRIBUTES)

#define UNPACK_ATTRIBUTE_AND_CALL_DOCUMENTATION_CONSTRUCTOR(r, data, elem) \
    tCommands.emplace_back(plato::input_parser::HelpDocumentation{         \
    BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(1, elem)),                      \
    BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(0, elem)),                      \
    BOOST_PP_TUPLE_ELEM(2, elem)}                                          \
    );

#define CREATE_STATIC_REGISTRATION_OF_DOCUMENTATION_FUNCTION(STRUCT_NAME, ATTRIBUTES)                                            \
namespace plato::input_parser {                                                                                                  \
template<>                                                                                                                       \
inline void register_help_documentation<STRUCT_NAME>() {                                                                         \
    [[maybe_unused]] const auto tRegistration = HelpDocumentationRegistration{ BOOST_PP_STRINGIZE(STRUCT_NAME),                  \
    [](){                                                                                                                        \
        Documentation tCommands;                                                                                                 \
        BOOST_PP_SEQ_FOR_EACH(UNPACK_ATTRIBUTE_AND_CALL_DOCUMENTATION_CONSTRUCTOR, _, BOOST_PP_VARIADIC_SEQ_TO_SEQ(ATTRIBUTES))  \
        return tCommands;                                                                                                        \
    }};                                                                                                                          \
};                                                                                                                               \
}

/// Macro for generating an adapted struct that can be used for input parsing. The format
/// is the same as BOOST_FUSION_DEFINE_STRUCT and the resulting struct has all the same
/// properties.
///
/// For example, a struct of the form:
/// @code 
/// PLATO_INPUT_BLOCK_STRUCT(
///    (plato)(input_parser), service, components::ComponentType::kProcessManager,
///    (unsigned int, number_processors))
/// @endcode
/// parses the following block:
/// @code
/// begin service my_service
///   number_processors 10
/// end service
/// @endcode
#define PLATO_INPUT_BLOCK_STRUCT(NAMESPACE_SEQ, STRUCT_NAME, COMPONENT_TYPE, ATTRIBUTES)                \
BOOST_FUSION_DEFINE_STRUCT(                                                                             \
    NAMESPACE_SEQ,                                                                                      \
    STRUCT_NAME,                                                                                        \
    TYPES_AS_OPTIONAL(ATTRIBUTES_WITHOUT_COMMENTS(BOOST_PP_VARIADIC_SEQ_TO_SEQ(ATTRIBUTES)))            \
)                                                                                                       \
CREATE_STATIC_REGISTRATION_OF_DOCUMENTATION_FUNCTION(STRUCT_NAME,  ATTRIBUTES)                          \
namespace plato::input_parser{                                                                          \
template<>                                                                                              \
struct InputTypeName<STRUCT_NAME>                                                                       \
{   static constexpr const char* name = #STRUCT_NAME;                                                   \
};                                                                                                      \
template<>                                                                                              \
struct ComponentTypeOfInputBlock<STRUCT_NAME>                                                           \
{                                                                                                       \
    constexpr static inline components::ComponentType value = COMPONENT_TYPE;                                       \
};                                                                                                      \
} 

#define PLATO_GEOMETRY_INPUT_BLOCK_STRUCT(NAMESPACE_SEQ, STRUCT_NAME, ATTRIBUTES)                      \
PLATO_INPUT_BLOCK_STRUCT(NAMESPACE_SEQ, STRUCT_NAME, components::ComponentType::kGeometry, ATTRIBUTES) \

#define PLATO_PROCESS_MANAGER_INPUT_BLOCK_STRUCT(NAMESPACE_SEQ, STRUCT_NAME, ATTRIBUTES)                     \
PLATO_INPUT_BLOCK_STRUCT(NAMESPACE_SEQ, STRUCT_NAME, components::ComponentType::kProcessManager, ATTRIBUTES) \

#define PLATO_FILTER_INPUT_BLOCK_STRUCT(NAMESPACE_SEQ, STRUCT_NAME, ATTRIBUTES)                         \
PLATO_INPUT_BLOCK_STRUCT(NAMESPACE_SEQ, STRUCT_NAME, components::ComponentType::kFilter, ATTRIBUTES)    \

/// Macro for generating an adapted struct that can be used for input parsing. The format
/// is the same as BOOST_FUSION_DEFINE_STRUCT and the resulting struct has all the same
/// properties. This will include a `name` field used to parse a block name.
///
/// For example, a struct of the form:
/// @code 
/// PLATO_NAMED_INPUT_BLOCK_STRUCT(
///    (plato)(input_parser), service, plato::componenets::ComponentType::kObjective
///    (unsigned int, number_processors))
/// @endcode
/// parses the following block:
/// @code
/// begin service my_service
///   number_processors 10
/// end service
/// @endcode
#define PLATO_NAMED_INPUT_BLOCK_STRUCT(NAMESPACE_SEQ, STRUCT_NAME, COMPONENT_TYPE, ATTRIBUTES)  \
BOOST_FUSION_DEFINE_STRUCT(                                                                     \
    NAMESPACE_SEQ,                                                                              \
    STRUCT_NAME,                                                                                \
    TYPES_AS_OPTIONAL(ATTRIBUTES_WITH_NAME(BOOST_PP_VARIADIC_SEQ_TO_SEQ(ATTRIBUTES)))           \
)                                                                                               \
CREATE_STATIC_REGISTRATION_OF_DOCUMENTATION_FUNCTION(STRUCT_NAME,  ATTRIBUTES)                  \
namespace plato::input_parser{                                                                  \
template<>                                                                                      \
struct InputTypeName<STRUCT_NAME>                                                               \
{   static constexpr const char* name = #STRUCT_NAME;                                           \
};                                                                                              \
template<>                                                                                      \
constexpr inline bool kIsNamedBlock<STRUCT_NAME> = true;                                        \
template<>                                                                                      \
struct ComponentTypeOfInputBlock<STRUCT_NAME>                                                   \
{                                                                                               \
    constexpr static inline components::ComponentType value = COMPONENT_TYPE;                   \
};                                                                                              \
}

// clang-format on

#endif
