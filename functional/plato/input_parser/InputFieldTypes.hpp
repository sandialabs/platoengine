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
    constexpr auto operator()() const -> const char* { return "-a-zA-Z_"; }
};

/// @brief Provides valid characters for an identifier, which are hyphen, underscore, all letters, and all
/// numbers.
struct ValidIdentifierCharacters
{
    constexpr auto operator()() const -> const char* { return "-a-zA-Z0-9_"; }
};

using AppName = UserDefinedToken<ValidNameCharacters>;
using CriterionName = UserDefinedToken<ValidNameCharacters>;
using BlockName = UserDefinedToken<ValidNameCharacters>;
using BlockList = UserDefinedTokenList<ValidIdentifierCharacters>;
using IdentifierString = UserDefinedToken<ValidIdentifierCharacters>;

[[nodiscard]] auto data_type_documentation(const AppName& /*unused*/) -> std::string;
[[nodiscard]] auto data_type_documentation(const BlockList& /*unused*/) -> std::string;
[[nodiscard]] auto data_type_documentation(const IdentifierString& /*unused*/) -> std::string;

}  // namespace plato::input_parser

#endif
