#ifndef PLATO_INPUTPARSER_POINT
#define PLATO_INPUTPARSER_POINT

#include <boost/fusion/include/define_struct.hpp>
#include <boost/spirit/include/qi.hpp>

///@brief Input struct with parser to read in a cartesian point from the input deck
// clang-format off
BOOST_FUSION_DEFINE_STRUCT(
(plato)(input_parser),Point,
(double, mX)
(double, mY)
(double, mZ)
)
// clang-format on

namespace plato::input_parser
{

[[nodiscard]] constexpr inline bool operator==(const Point& aLHS, const Point& aRHS)
{
    return aLHS.mX == aRHS.mX && aLHS.mY == aRHS.mY && aLHS.mZ == aRHS.mZ;
}

}  // namespace plato::input_parser

namespace boost::spirit::traits
{

///@brief Specialization of the parser for the type Point. Reads in a Point of the form "( x , y , z )".
template <typename Iterator>
struct create_parser<plato::input_parser::Point, Iterator>
{
    static auto call()
    {
        return boost::proto::deep_copy(boost::spirit::qi::lit('(') >> boost::spirit::qi::double_ >>
                                       boost::spirit::qi::lit(',') >> boost::spirit::qi::double_ >>
                                       boost::spirit::qi::lit(',') >> boost::spirit::qi::double_ >>
                                       boost::spirit::qi::lit(')'));
    }
    using type = decltype(call());
};

}  // namespace boost::spirit::traits

#endif
