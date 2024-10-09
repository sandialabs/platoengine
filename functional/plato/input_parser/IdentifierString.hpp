#ifndef PLATO_INPUT_PARSER_IDENTIFIERSTRING
#define PLATO_INPUT_PARSER_IDENTIFIERSTRING

#include "plato/input_parser/UserDefinedToken.hpp"

namespace plato::input_parser
{
/// @brief Valid characters for a string that does not contain special characters except dash and underscore.
struct ValidIdentifierCharacters
{
    constexpr const char* operator()() { return "-a-zA-Z0-9_"; }
};

using IdentifierString = UserDefinedToken<ValidIdentifierCharacters>;

}  // namespace plato::input_parser

#endif
