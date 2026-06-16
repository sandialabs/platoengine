#include <gtest/gtest.h>

#include <boost/phoenix/core/reference.hpp>
#include <boost/phoenix/stl/container/container.hpp>
#include <boost/spirit/home/qi/action/action.hpp>
#include <boost/spirit/home/qi/string/symbols.hpp>
#include <boost/spirit/home/support/common_terminals.hpp>
#include <cstdint>
#include <string>
#include <vector>

#include "plato/input_parser/EnumParser.hpp"
#include "plato/utilities/EnumTable.hpp"

namespace plato::input_parser::unittest
{
namespace
{
enum struct Pizza : std::uint8_t
{
    kCheese,
    kSupreme,
    kPepperoni
};

const utilities::EnumTable<Pizza> kPizzaTable({{Pizza::kCheese, "cheese"},
                                               {Pizza::kSupreme, "supreme"},
                                               {Pizza::kPepperoni, "pepperoni"}});

template <typename Iterator>
auto parse_pizza_list(Iterator aBegin, Iterator aEnd, std::vector<Pizza>& aPizzas) -> bool
{
    namespace bsq = boost::spirit::qi;
    namespace bsa = boost::spirit::ascii;
    namespace bp = boost::phoenix;

    const boost::spirit::qi::symbols<char, Pizza> tSymbolTable = make_enum_symbols<Pizza>(kPizzaTable);

    const bool tResult =
        bsq::phrase_parse(aBegin, aEnd, (tSymbolTable[bp::push_back(bp::ref(aPizzas), bsq::_1)] % ','), bsa::space);

    if (aBegin != aEnd)
    {
        return false;
    }
    return tResult;
}
}  // namespace

TEST(BoostSpirit, ParseEnum)
{
    std::vector<Pizza> tParsedPizzas;

    const std::string tValidPizzas = "cheese, pepperoni,  supreme,    cheese";
    EXPECT_TRUE(parse_pizza_list(tValidPizzas.begin(), tValidPizzas.end(), tParsedPizzas));
    EXPECT_EQ(tParsedPizzas.size(), 4U);
}
}  // namespace plato::input_parser::unittest
