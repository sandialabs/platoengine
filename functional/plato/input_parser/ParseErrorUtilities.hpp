#ifndef PLATO_INPUT_PARSER_PARSEERRORUTILITIES
#define PLATO_INPUT_PARSER_PARSEERRORUTILITIES

#include <algorithm>
#include <string>

namespace plato::input_parser
{
/// @brief Checks that a parser error has occurred.
/// @param aParsedSuccessfully The return value from `boost::spirit::qi::phrase_parse`.
/// @param aParsedUntilIterator The returned iterator that gives the position in the input the parser was able to parse
/// to.
/// @param aInputEndIterator The end of the input iterator.
template <typename Iterator>
auto parser_has_error(bool aParsedSuccessfully, Iterator aParsedUntilIterator, Iterator aInputEndIterator) -> bool;

/// @brief Returns an error message for a parsing error that contains some input context.
template <typename Iterator>
auto error_message(Iterator aParsedUntilIterator, Iterator aInputEndIterator, char aTokenDelimiter) -> std::string;

template <typename Iterator>
auto parser_has_error(const bool aParsedSuccessfully,
                      const Iterator aParsedUntilIterator,
                      const Iterator aInputEndIterator) -> bool
{
    return !aParsedSuccessfully || aParsedUntilIterator != aInputEndIterator;
}

template <typename Iterator>
auto error_message(const Iterator aParsedUntilIterator, const Iterator aInputEndIterator, const char aTokenDelimiter)
    -> std::string
{
    const auto tToDelimeterIterator = std::find(aParsedUntilIterator, aInputEndIterator, aTokenDelimiter);
    const auto tContext = std::string{aParsedUntilIterator, tToDelimeterIterator};
    return "Parsing error near: \n" + tContext;
}
}  // namespace plato::input_parser

#endif
