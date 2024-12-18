#ifndef PLATO_INPUTPARSER_DOCUMENTATIONGENERATOR_LATEXFORMATTER
#define PLATO_INPUTPARSER_DOCUMENTATIONGENERATOR_LATEXFORMATTER

#include <map>
#include <string>
#include <vector>

#include "plato/input_parser/HelpDocumentationRegistration.hpp"

namespace plato::input_parser::documentation_generator::library
{
using BlockCommands = std::map<std::string, std::vector<std::string>>;
using HelpDocumentationMap = std::map<std::string, Documentation>;

///@brief Take help documentation @a aHelpDocumentation and convert it from vectors of functions to vectors of the
/// string data that is formatted into a latex command that can be defined and adapted as we need.
[[nodiscard]] auto format_command_latex(const HelpDocumentationMap& aHelpDocumentationMap) -> BlockCommands;

}  // namespace plato::input_parser::documentation_generator::library

#endif
