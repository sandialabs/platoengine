#ifndef PLATO_INPUTPARSER_RANGE
#define PLATO_INPUTPARSER_RANGE

#include <boost/fusion/include/define_struct.hpp>
#include <boost/spirit/include/qi.hpp>

///@brief Input struct with parser to read in inclusive bounds from the input deck
// clang-format off
BOOST_FUSION_DEFINE_STRUCT(
(plato)(input_parser),Range,
(double, mLower)
(double, mStep)
(double, mUpper)
)
// clang-format on

namespace plato::input_parser
{

[[nodiscard]] constexpr inline bool operator==(const Range& aLHS, const Range& aRHS)
{
    return aLHS.mLower == aRHS.mLower && aLHS.mStep == aRHS.mStep && aLHS.mUpper == aRHS.mUpper;
}

}  // namespace plato::input_parser

namespace boost::spirit::traits
{

///@brief Specialization of the parser for the type Range. Reads in a range of the form "[ lower : step : upper ]".
template <typename Iterator>
struct create_parser<plato::input_parser::Range, Iterator>
{
    static auto call()
    {
        return boost::proto::deep_copy(boost::spirit::qi::lit('[') >> boost::spirit::qi::double_ >>
                                       boost::spirit::qi::lit(':') >> boost::spirit::qi::double_ >>
                                       boost::spirit::qi::lit(':') >> boost::spirit::qi::double_ >>
                                       boost::spirit::qi::lit(']'));
    }
    using type = decltype(call());
};

}  // namespace boost::spirit::traits

#endif
