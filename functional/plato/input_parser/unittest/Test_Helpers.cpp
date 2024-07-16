#define BOOST_PHOENIX_STL_TUPLE_H_  // Work-around for ODR violation in boost phoenix.
                                    // https://github.com/boostorg/phoenix/issues/111

#include "plato/input_parser/unittest/Test_Helpers.hpp"

#include <string>
#include <tuple>

#include "plato/input_parser/InputBlocks.hpp"
#include "plato/input_parser/InputParser.hpp"
#include "plato/input_parser/Skipper.hpp"

namespace plato::input_parser::unittest
{
auto parse_string(const std::string& aInput) -> std::tuple<bool, std::string::const_iterator, ParsedInput>
{
    using Iterator = std::string::const_iterator;
    InputParser<Iterator> tParser;
    ParsedInput tData;
    auto tIter = aInput.cbegin();
    const auto tSkipper = SkipperRule<Iterator>{};
    const bool tParseResult = phrase_parse(tIter, aInput.cend(), tParser, tSkipper.skipperRule(), tData);
    return {tParseResult, tIter, tData};
}
}  // namespace plato::input_parser::unittest