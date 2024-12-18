#ifndef PLATO_INPUTPARSER_HELPDOCUMENTATIONREGISTRATION
#define PLATO_INPUTPARSER_HELPDOCUMENTATIONREGISTRATION

#include <functional>
#include <map>
#include <string>
#include <vector>

namespace plato::input_parser
{

/// @brief Helper struct used to hold the string forms of a plato input block
/// @note Not indented for use besides documentation.
struct HelpDocumentation
{
    std::string mName = "Default Name";
    std::string mType = "Default Type";
    std::string mComment = "Default Comment";
};

using Documentation = std::vector<HelpDocumentation>;
using HelpDocumentationFunction = std::function<Documentation()>;

/// @brief Object used for static registration of help documentation functions
///
/// To register functions, client code should instantiate a static object in a cpp file.
/// For example,
/// @code
/// namespace{
/// [[maybe_unused]] static auto kNewHelpDocumentationRegistration =
///   plato::input_parser::HelpDocumentationRegistration{
///    [](){ return help_documentation_function(); }
/// };
/// }
/// @endcode
///
struct HelpDocumentationRegistration
{
    HelpDocumentationRegistration(const std::string& aKey, HelpDocumentationFunction aFunction);
};

/// @brief Generates the list of all documentation available
[[nodiscard]] auto generate_documentation() -> std::map<std::string, Documentation>;

/// @brief Generate the list of documentation and format the text and print it in the given output stream @a
/// aOutputStream
void known_inputs(std::ostream& aOutputStream);

namespace detail
{

/// @brief Creates and/or retrieves the static map that will hold the documentation functions
[[nodiscard]] auto registered_help_documentation_functions() -> std::map<std::string, HelpDocumentationFunction>&;

}  // namespace detail

}  // namespace plato::input_parser

#endif
