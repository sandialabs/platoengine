#ifndef PLATO_INPUT_PARSER_INPUTFIELDTYPES
#define PLATO_INPUT_PARSER_INPUTFIELDTYPES

#include "plato/input_parser/UserDefinedToken.hpp"
#include "plato/input_parser/UserDefinedTokenList.hpp"

namespace plato::input_parser
{
/// @brief Provides valid characters for an app's name, which are hyphen, underscore,
///  and all letters.
struct ValidAppNameCharacters
{
    constexpr const char* operator()() const { return "-a-zA-Z_"; }
};

/// @brief Provides valid characters for a mesh block's name, which are hyphen, underscore, all letters, and all
/// numbers.
struct ValidBlockNameCharacters
{
    constexpr const char* operator()() const { return "-a-zA-Z0-9_"; }
};

using AppName = UserDefinedToken<ValidAppNameCharacters>;
using CriterionName = UserDefinedToken<ValidAppNameCharacters>;
using FixedBlockList = UserDefinedTokenList<ValidBlockNameCharacters>;

}  // namespace plato::input_parser

#endif
