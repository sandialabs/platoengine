#include <gtest/gtest.h>

#include <boost/phoenix/core.hpp>
#include <boost/phoenix/operator.hpp>
#include <boost/phoenix/stl/algorithm.hpp>
#include <boost/phoenix/stl/container.hpp>

#include "plato/input_parser/EnumParser.hpp"
#include "plato/utilities/EnumTable.hpp"

namespace plato::input_parser::unittest
{
namespace
{
enum struct Pizza
{
    kCheese,
    kSupreme,
    kPepperoni
};

const utilities::EnumTable<Pizza> kPizzaTable({{Pizza::kCheese, "cheese"},
                                               {Pizza::kSupreme, "supreme"},
                                               {Pizza::kPepperoni, "pepperoni"}});

template <typename Iterator>
bool parse_pizza_list(Iterator aBegin, Iterator aEnd, std::vector<Pizza>& aPizzas)
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
    std::vector<Pizza> parsedPizzas;

    const std::string validPizzas = "cheese, pepperoni,  supreme,    cheese";
    EXPECT_TRUE(parse_pizza_list(validPizzas.begin(), validPizzas.end(), parsedPizzas));
    EXPECT_EQ(parsedPizzas.size(), 4);
}
}  // namespace plato::input_parser::unittest