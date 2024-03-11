#define BOOST_PHOENIX_STL_TUPLE_H_  // Work-around for ODR violation in boost phoenix.
                                    // https://github.com/boostorg/phoenix/issues/111

#include "plato/input_parser/InputParser.hpp"

#include <fstream>
#include <streambuf>

#include "plato/utilities/Exception.hpp"

namespace plato::input_parser
{
ParsedInput parse_input(const std::string_view aInput)
{
    InputParser<std::string_view::const_iterator> tParser;
    ParsedInput tData;
    auto tIter = aInput.cbegin();
    const bool tParseResult = phrase_parse(tIter, aInput.cend(), tParser, boost::spirit::ascii::space, tData);
    if (!tParseResult || tIter != aInput.cend())
    {
        throw utilities::Exception("Could not parse input deck.");
    }
    return tData;
}

ParsedInput parse_input_from_file(const std::filesystem::path& aFileName)
{
    std::ifstream tInputStream(aFileName);
    const std::string tInputFileString((std::istreambuf_iterator<char>(tInputStream)),
                                       std::istreambuf_iterator<char>());
    return parse_input(tInputFileString);
}

}  // namespace plato::input_parser
