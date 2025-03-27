#include "plato/input_parser/GenericBlockRule.hpp"

namespace plato::input_parser
{
auto parse_generic_blocks(const std::string_view aInput) -> std::vector<GenericBlockData>
{
    using Iterator = std::string_view::const_iterator;

    auto tIter = aInput.begin();
    auto tData = std::vector<GenericBlockData>{};
    const auto tParser = GenericBlockParser<Iterator>{};
    const auto tSkipper = SkipperRule<Iterator>{};
    [[maybe_unused]] const auto tParseResult = boost::spirit::qi::phrase_parse(
        tIter, aInput.cend(), +(tParser.mRule), tSkipper.skipperRule(), tData);  // FIX-ME, Add error checking

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
