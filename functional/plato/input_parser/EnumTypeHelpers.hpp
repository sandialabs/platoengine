#ifndef PLATO_INPUT_PARSER_ENUMTYPEHELPERS
#define PLATO_INPUT_PARSER_ENUMTYPEHELPERS

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/variadic_seq_to_seq.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/spirit/include/qi.hpp>
#include <iostream>

#include "plato/input_parser/EnumParser.hpp"
#include "plato/utilities/EnumTable.hpp"

// clang-format off
#define ENUMERATE(a) BOOST_PP_TUPLE_ELEM(2, 0, a)
#define ENUM_STRING(a) BOOST_PP_TUPLE_ELEM(2, 1, a)
#define ENUMERATE_WITH_COMMA(r,data,a) ENUMERATE(a),

#define TABLE_NAME_FROM_ENUM_NAME(ENUM_NAME) BOOST_PP_CAT(BOOST_PP_CAT(k,ENUM_NAME),Table)

#define ENUM_STRUCT_DEFINE(ENUM_NAME, SEQ)                                                      \
enum struct ENUM_NAME {                                                                         \
BOOST_PP_SEQ_FOR_EACH(ENUMERATE_WITH_COMMA, _, SEQ)                                             \
};                                                                                              

#define ENUM_TABLE_ENTRY(r,ENUM_NAME, tuple)                                                    \
{ENUM_NAME::ENUMERATE(tuple), ENUM_STRING(tuple)},

#define ENUM_TABLE_DEFINE(ENUM_NAME, SEQ)                                                       \
static const plato::utilities::EnumTable<ENUM_NAME>                                 \
TABLE_NAME_FROM_ENUM_NAME(ENUM_NAME)                                                            \
({                                                                                              \
BOOST_PP_SEQ_FOR_EACH(ENUM_TABLE_ENTRY, ENUM_NAME, SEQ)                                         \
});

#define PLATO_ENUM_STRUCT_TABLE_AND_OUTPUT_OPERATOR(ENUM_NAME, SEQ)                             \
namespace plato::input_parser                                                       \
{                                                                                               \
    ENUM_STRUCT_DEFINE(ENUM_NAME, SEQ)                                                          \
    ENUM_TABLE_DEFINE(ENUM_NAME, SEQ)                                                           \
    std::ostream& operator<<(std::ostream& aStream, ENUM_NAME aOption);                         \
}

#define BOOST_SPIRIT_TRAITS_CREATE_PARSER(ENUM_NAME)                                            \
namespace boost::spirit::traits                                                                 \
{                                                                                               \
template <>                                                                                     \
struct create_parser<plato::input_parser::ENUM_NAME>                                \
{                                                                                               \
    static const boost::spirit::qi::symbols<char, plato::input_parser::ENUM_NAME>   \
                                            mSymbolTable;                                       \
    using type = typename boost::proto::result_of::deep_copy<BOOST_TYPEOF(mSymbolTable)>::type; \
    static type call()                                                                          \
    {                                                                                           \
        return boost::proto::deep_copy(mSymbolTable);                                           \
    }                                                                                           \
};                                                                                              \
}

/// Macro for declaring an `enum` that has a matching, parsable symbol table. The
/// first argument is the enum type and the second argument is a list of enumerate/string
/// pairs.
/// This creates three declarations: an `enum struct`, an EnumTable, and an `boost::spirit::qi::symbols`
/// object.
/// Usage is:
/// @code 
/// DECLARE_ENUM_SYMBOL_TABLE(
///    NewEnum,
///    (kFooEnumerate, "foo")
///    (kBarEnumerate, "bar"))
/// @endcode
/// @note This must be used in a header file, along with a matching DEFINE_ENUM_SYMBOL_TABLE in 
///  a cpp file.
#define DECLARE_ENUM_SYMBOL_TABLE(ENUM_NAME, SEQ)                                                               \
PLATO_ENUM_STRUCT_TABLE_AND_OUTPUT_OPERATOR(ENUM_NAME, BOOST_PP_VARIADIC_SEQ_TO_SEQ(SEQ))                       \
BOOST_SPIRIT_TRAITS_CREATE_PARSER(ENUM_NAME)

/// Defines the EnumTable and boost spirit symbol table. Must be in a cpp file and have a matching
/// DECLARE_ENUM_SYMBOL_TABLE in a header.
/// Usage is:
/// @code
/// DEFINE_ENUM_SYMBOL_TABLE(NewEnum)
/// @endcode
/// and `NewEnum` must have a matching DECLARE_ENUM_SYMBOL_TABLE.
#define DEFINE_ENUM_SYMBOL_TABLE(ENUM_NAME)                                                                     \
namespace boost::spirit::traits {                                                                               \
const boost::spirit::qi::symbols<char, plato::input_parser::ENUM_NAME>                              \
    create_parser<plato::input_parser::ENUM_NAME>::mSymbolTable =                                   \
            plato::input_parser::make_enum_symbols<plato::input_parser::ENUM_NAME>      \
            (plato::input_parser::TABLE_NAME_FROM_ENUM_NAME(ENUM_NAME));                            \
}                                                                                                               \
namespace plato::input_parser                                                                       \
{                                                                                                               \
    std::ostream& operator<<(std::ostream& aStream, const ENUM_NAME aOption)                                    \
    {                                                                                                           \
        aStream << *TABLE_NAME_FROM_ENUM_NAME(ENUM_NAME).toString(aOption);                                     \
        return aStream;                                                                                         \
    }                                                                                                           \
}
// clang-format on

#endif
