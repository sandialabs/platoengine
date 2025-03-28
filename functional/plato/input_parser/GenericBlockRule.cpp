#include "plato/input_parser/GenericBlockRule.hpp"

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

    const auto tParseResult =
        boost::spirit::qi::phrase_parse(tInputIterator, aInput.cend(), +(tParser.mRule), tSkipper.skipperRule(), tData);

    if (!tParseResult || tInputIterator != aInput.cend())
    {
        const auto tNewLineIterator = std::find(tInputIterator, aInput.cend(), '\n');
        const auto tContext = std::string{tInputIterator, tNewLineIterator};
        return utilities::unexpected("Parsing error near: \n" + tContext);
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
