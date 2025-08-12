#include "plato/input_parser/documentation_generator/library/LatexFormatter.hpp"

#include <iterator>
#include <string_view>
#include <algorithm>

#include "plato/utilities/StringUtilities.hpp"

namespace plato::input_parser::documentation_generator::library
{

namespace
{
[[nodiscard]] auto enclose_in_curly_braces(const std::string& aString) -> std::string { return "{" + aString + "}"; }

constexpr auto kCommandPrefix = std::string_view{"\\PlatoCommand"};
[[nodiscard]] auto format_command(const std::string& aBlockName, const HelpDocumentation& aHelpDocumentation)
    -> std::string
{
    return utilities::concatenate(std::string{kCommandPrefix}, enclose_in_curly_braces(aBlockName),
                                  enclose_in_curly_braces(aHelpDocumentation.mName),
                                  enclose_in_curly_braces(aHelpDocumentation.mType),
                                  enclose_in_curly_braces(aHelpDocumentation.mComment));
}

[[nodiscard]] auto convert_to_command(const std::pair<std::string, Documentation>& aMapEntry)
    -> std::vector<std::string>
{
    std::vector<std::string> tFormattedCommands;
    tFormattedCommands.reserve(aMapEntry.second.size());
    std::transform(aMapEntry.second.begin(), aMapEntry.second.end(), std::back_inserter(tFormattedCommands),
                   [aMapEntry](const auto& aHelpDocumentation)
                   { return format_command(aMapEntry.first, aHelpDocumentation); });
    return tFormattedCommands;
}

}  // namespace

auto format_command_latex(const HelpDocumentationMap& aHelpDocumentationMap) -> BlockCommands
{
    BlockCommands tBlockCommands;
    for (const auto& tMapEntry : aHelpDocumentationMap)
    {
        tBlockCommands[tMapEntry.first] = convert_to_command(tMapEntry);
    }
    return tBlockCommands;
}

}  // namespace plato::input_parser::documentation_generator::library
