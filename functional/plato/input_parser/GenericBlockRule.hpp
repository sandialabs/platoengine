#ifndef PLATO_INPUT_PARSER_GENERICBLOCKRULE
#define PLATO_INPUT_PARSER_GENERICBLOCKRULE

#include <boost/fusion/adapted/struct/define_struct.hpp>
#include <boost/fusion/include/define_struct.hpp>
#include <boost/spirit/include/qi.hpp>

#include "plato/input_parser/InputFieldTypes.hpp"
#include "plato/input_parser/Skipper.hpp"
#include "plato/utilities/Expected.hpp"

namespace plato::input_parser
{
/// @brief A string token that may contain any printable (non-whitespace) character.
struct GenericToken
{
    using value_type = char;
    [[nodiscard]] std::string::const_iterator begin() const { return mToken.begin(); }
    [[nodiscard]] std::string::const_iterator end() const { return mToken.end(); }
    [[nodiscard]] std::string::iterator begin() { return mToken.begin(); }
    [[nodiscard]] std::string::iterator end() { return mToken.end(); }
    void insert(std::string::iterator aIter, char aVal) { mToken.insert(aIter, aVal); }

    std::string mToken;
};
}  // namespace plato::input_parser

namespace boost::spirit::traits
{
template <>
struct create_parser<plato::input_parser::GenericToken>
{
    typedef proto::result_of::deep_copy<BOOST_TYPEOF((qi::lexeme[+qi::graph] - qi::lit("end")))>::type type;

    static type call() { return proto::deep_copy((qi::lexeme[+qi::graph] - qi::lit("end"))); }
};
}  // namespace boost::spirit::traits

// clang-format off
BOOST_FUSION_DEFINE_STRUCT(
    (plato)(input_parser),
    GenericBlockData,
    (plato::input_parser::BlockName, mName)
    (std::vector<plato::input_parser::GenericToken>, mInput)
)
// clang-format on

namespace plato::input_parser
{

/// @brief A parser to parse and tokenize text delineated by `begin` and `end` tokens with a block type as the first
/// token after `begin`.
///
/// An example of valid input is:
/// @verbatim
/// begin block_type_token
///  arbitrary text
///  more input text
/// end
/// @endverbatim
/// The parser is white-space agnostic in that it doesn't expect newlines as given above (spaces or tabs are allowed).
/// This example will parse into a string (`block_type_token`) and a vector of five strings: `arbitrary`. `text`,
/// `more`, `arbitrary`, `text`.
template <typename Iterator>
struct GenericBlockParser : boost::spirit::qi::grammar<Iterator, GenericBlockData(), SkipperType<Iterator>>
{
    GenericBlockParser();

    boost::spirit::qi::rule<Iterator, GenericBlockData(), SkipperType<Iterator>> mRule;
};

/// @brief Parses the string @a aInput into a vector of GenericBlockData objects.
[[nodiscard]] auto parse_generic_blocks(std::string_view aInput)
    -> utilities::Expected<std::vector<GenericBlockData>, std::string>;

/// @brief Converts the tokenized input held by @a aData to a string.
[[nodiscard]] auto to_string(const GenericBlockData& aData) -> std::string;

template <typename Iterator>
GenericBlockParser<Iterator>::GenericBlockParser()
    : GenericBlockParser::base_type{mRule},
      mRule{boost::spirit::qi::lit("begin") >> boost::spirit::qi::auto_ >> +(boost::spirit::qi::auto_) >>
            boost::spirit::qi::lit("end")}
{
}

}  // namespace plato::input_parser

#endif
