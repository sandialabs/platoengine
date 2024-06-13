#ifndef PLATO_INPUT_PARSER_SKIPPER
#define PLATO_INPUT_PARSER_SKIPPER

#include <boost/spirit/home/qi/nonterminal/rule.hpp>
#include <boost/spirit/include/qi.hpp>

namespace plato::input_parser
{
/// @brief The skipper grammar for the parser. This enables the parser to skip whitespace and comments.
template <typename Iterator>
class SkipperRule
{
   public:
    using Rule = boost::spirit::qi::rule<Iterator>;

    const Rule& skipperRule() const { return mAllRules; }

   private:
    constexpr static std::string_view kCommentCharacter = "#";

    Rule mWhiteSpaceSkipper = boost::spirit::qi::ascii::space;
    Rule mCommentSkipper = kCommentCharacter.data() >> *(boost::spirit::qi::char_ - boost::spirit::qi::eol) >>
                           (boost::spirit::qi::eol | boost::spirit::qi::eoi);
    Rule mAllRules = mWhiteSpaceSkipper | mCommentSkipper;
};

template <typename Iterator>
using SkipperType = typename SkipperRule<Iterator>::Rule;

}  // namespace plato::input_parser

#endif
