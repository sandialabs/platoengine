#include "plato/input_parser/GenericBlockRule.hpp"

#include <boost/spirit/home/qi/parse.hpp>
#include <numeric>
#include <string>
#include <string_view>
#include <vector>

#include "plato/input_parser/ParseErrorUtilities.hpp"
#include "plato/input_parser/Skipper.hpp"
#include "plato/utilities/Expected.hpp"

namespace plato::input_parser
{
auto parse_generic_blocks(const std::string_view aInput)
    -> utilities::Expected<std::vector<GenericBlockData>, std::string>
{
    using Iterator = std::string_view::const_iterator;

    auto tInputIterator = aInput.begin();
    auto tData = std::vector<GenericBlockData>{};
    const auto tParser = GenericBlockParser<Iterator>{};
    const auto tSkipper = SkipperRule<Iterator>{};

    const auto tParserSuccessful =
        boost::spirit::qi::phrase_parse(tInputIterator, aInput.cend(), +(tParser.mRule), tSkipper.skipperRule(), tData);

    if (parser_has_error(tParserSuccessful, tInputIterator, aInput.cend()))
    {
        constexpr auto tDelimeter = '\n';
        return utilities::unexpected(error_message(tInputIterator, aInput.cend(), tDelimeter));
    }
    return tData;
}

auto to_string(const GenericBlockData& aData) -> std::string
{
    return std::accumulate(aData.mInput.begin(), aData.mInput.end(), std::string{},
                           [](std::string aConcatenatedString, const GenericToken& aNewToken)
                           {
                               aConcatenatedString += aNewToken.mToken;
                               aConcatenatedString += " ";
                               return aConcatenatedString;
                           });
}
}  // namespace plato::input_parser
