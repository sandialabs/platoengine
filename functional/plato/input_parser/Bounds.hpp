#ifndef PLATO_INPUTPARSER_BOUNDS
#define PLATO_INPUTPARSER_BOUNDS

#include <boost/fusion/include/define_struct.hpp>
#include <boost/spirit/include/qi.hpp>

///@brief Input struct with parser to read in inclusive bounds from the input deck
// clang-format off
BOOST_FUSION_DEFINE_STRUCT(
(plato)(input_parser),Bounds,
(double, mLower)
(double, mUpper)
)
// clang-format on

namespace boost::spirit::traits
{

///@brief Specialization of the parser for the type Bounds. Reads in a bounds of the form "[ lower , upper ]".
template <typename Iterator>
struct create_parser<plato::input_parser::Bounds, Iterator>
{
    static auto call()
    {
        return boost::proto::deep_copy(boost::spirit::qi::lit('[') >> boost::spirit::qi::double_ >>
                                       boost::spirit::qi::lit(',') >> boost::spirit::qi::double_ >>
                                       boost::spirit::qi::lit(']'));
    }
    using type = decltype(call());
};

}  // namespace boost::spirit::traits

#endif
