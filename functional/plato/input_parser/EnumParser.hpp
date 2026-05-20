#ifndef PLATO_INPUT_PARSER_ENUMPARSER
#define PLATO_INPUT_PARSER_ENUMPARSER

#include <boost/spirit/include/qi.hpp>

#include "plato/utilities/EnumTable.hpp"

namespace plato::input_parser
{
template <typename Enum>
auto make_enum_symbols(const utilities::EnumTable<Enum>& aTable) -> boost::spirit::qi::symbols<char, Enum>
{
    const boost::spirit::qi::symbols<char, Enum> tSymbols;
    for (const auto& tEntry : aTable)
    {
        tSymbols.add(tEntry.second, tEntry.first);
    }
    return tSymbols;
}

}  // namespace plato::input_parser

#endif
