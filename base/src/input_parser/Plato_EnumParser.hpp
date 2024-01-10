#ifndef PLATO_ENUMPARSER_HPP
#define PLATO_ENUMPARSER_HPP

#include "Plato_EnumTable.hpp"

#include <boost/spirit/include/qi.hpp>

namespace Plato{

template<typename Enum>
auto make_enum_symbols(const Plato::EnumTable<Enum>& aTable) -> boost::spirit::qi::symbols<char, Enum>
{
    boost::spirit::qi::symbols<char, Enum> tSymbols;
    for(const auto& entry : aTable)
    {
        tSymbols.add(entry.second, entry.first);
    }
    return tSymbols;
}

}

#endif
