#ifndef PLATO_INPUT_PARSER_INPUTFIELDTYPES
#define PLATO_INPUT_PARSER_INPUTFIELDTYPES

#include "plato/input_parser/UserDefinedToken.hpp"

namespace plato::input_parser
{
/// @brief Provides valid characters for an app's name, which are hyphen, underscore,
///  and all letters.
struct ValidAppNameCharacters
{
    constexpr const char* operator()() const { return "-a-zA-Z_"; }
};
using AppName = UserDefinedToken<ValidAppNameCharacters>;
using CriterionName = UserDefinedToken<ValidAppNameCharacters>;
}  // namespace plato::input_parser

#endif
