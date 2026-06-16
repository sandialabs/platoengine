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

    auto skipperRule() const -> const Rule& { return mAllRules; }

   private:
    constexpr static std::string_view mCommentCharacter = "#";
    constexpr static std::string_view mBlockCommentStartCharacters = "/*";
    constexpr static std::string_view mBlockCommentEndCharacters = "*/";

    Rule mWhiteSpaceSkipper = boost::spirit::qi::ascii::space;
    Rule mCommentSkipper = mCommentCharacter.data() >> *(boost::spirit::qi::char_ - boost::spirit::qi::eol) >>
                           (boost::spirit::qi::eol | boost::spirit::qi::eoi);
    Rule mBlockCommentSkipper =
        boost::spirit::qi::lexeme[mBlockCommentStartCharacters.data() >>
                                  *(boost::spirit::qi::char_ - mBlockCommentEndCharacters.data()) >>
                                  mBlockCommentEndCharacters.data()];
    Rule mAllRules = mWhiteSpaceSkipper | mCommentSkipper | mBlockCommentSkipper;
};

template <typename Iterator>
using SkipperType = typename SkipperRule<Iterator>::Rule;

}  // namespace plato::input_parser

#endif
