#ifndef PLATO_INPUT_PARSER_INPUTFIELDTYPES
#define PLATO_INPUT_PARSER_INPUTFIELDTYPES

#include "plato/input_parser/UserDefinedToken.hpp"
#include "plato/input_parser/UserDefinedTokenList.hpp"

namespace plato::input_parser
{
/// @brief Provides valid characters for a name, which are hyphen, underscore, and all letters.
///
/// This should be used for the names of input blocks and apps in a plato input deck.
struct ValidNameCharacters
{
    constexpr const char* operator()() const { return "-a-zA-Z_"; }
};

/// @brief Provides valid characters for an identifier, which are hyphen, underscore, all letters, and all
/// numbers.
struct ValidIdentifierCharacters
{
    constexpr const char* operator()() const { return "-a-zA-Z0-9_"; }
};

using AppName = UserDefinedToken<ValidNameCharacters>;
using CriterionName = UserDefinedToken<ValidNameCharacters>;
using BlockName = UserDefinedToken<ValidNameCharacters>;
using FixedBlockList = UserDefinedTokenList<ValidIdentifierCharacters>;
using IdentifierString = UserDefinedToken<ValidIdentifierCharacters>;

}  // namespace plato::input_parser

#endif
