#ifndef PLATO_GEOMETRY_EXTENSION_CUBIT_BOUNDEDAPREPROVARIABLEPARSER
#define PLATO_GEOMETRY_EXTENSION_CUBIT_BOUNDEDAPREPROVARIABLEPARSER

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/define_struct.hpp>
#include <boost/proto/deep_copy.hpp>
#include <boost/spirit/include/qi.hpp>

#include "plato/input_parser/AutoList.hpp"
#include "plato/input_parser/Bounds.hpp"
#include "plato/input_parser/UserDefinedToken.hpp"

namespace plato::input_parser
{
/// @brief Valid characters for an aprepro variable are alpha numeric with underscore.
struct ValidApreproCharacters
{
    constexpr const char* operator()() const { return "a-zA-Z0-9_"; }
};

using ApreproString = UserDefinedToken<ValidApreproCharacters>;

/// @brief a bounded aprepro variable that can be read in from the input
struct BoundedApreproVariable
{
    ApreproString mName;
    double mInitialValue;
    Bounds mBounds;
};

}  // namespace plato::input_parser

// clang-format off
BOOST_FUSION_ADAPT_STRUCT(
plato::input_parser::BoundedApreproVariable,
(plato::input_parser::ApreproString, mName)
(double, mInitialValue)
(plato::input_parser::Bounds, mBounds)
)
// clang-format on

namespace boost::spirit::traits
{

/// @brief Specialization of create_parser for the BoundedApreproVariable type.
template <typename Iterator>
struct create_parser<plato::input_parser::BoundedApreproVariable, Iterator>
{
    static auto call()
    {
        return boost::proto::deep_copy(boost::spirit::qi::auto_ >> boost::spirit::qi::double_ >>
                                       boost::spirit::qi::auto_);
    }
    using type = decltype(call());
};

}  // namespace boost::spirit::traits

namespace plato::input_parser
{
using BoundedApreproVariableList = AutoList<BoundedApreproVariable>;
}

#endif
